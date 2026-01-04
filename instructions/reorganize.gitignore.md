# Reorganize .gitignore

This repository expects a stable, predictable layout for `.gitignore`.
When reorganizing the file, apply the rules below in order and do not
compress or shorten them. The goal is to ensure ignore behavior stays
intentional, easy to scan, and easy to diff over time.

## Scope

These rules apply to the root `.gitignore` and should be followed anytime
an agent (human or AI) reorders or normalizes entries. If the file already
contains a custom note or comment, preserve it and keep it with the block
it describes.

## Ordering Rules (Strict)

1. **Alphabetical ascending order**:
   - Sort by the literal text of each pattern, using plain ASCII ordering.
   - Include the leading `/` or `.` when determining the order.
   - Do not ignore case; keep exact-case ordering (ASCII sort is stable).
   - Dot-prefixed entries (`/.foo/`, `/.bar`) naturally sort before
     non-dot names; keep that ordering even if the non-dot entry is
     "more important" in your judgement.
   - Path depth does not change ordering; always sort by the full literal
     string. Shorter paths with a shared prefix sort before deeper ones
     (e.g., `/docs/` before `/docs/source/`).

2. **Overrides immediately follow the pattern they override**:
   - Any un-ignore rule (`!path`) must appear directly after the pattern
     that would otherwise ignore it.
   - This local adjacency rule overrides global alphabetical ordering.
   - If multiple override lines apply to a single ignore line, keep them
     together as a block immediately after the base rule.

## Normalization Rules (Path Semantics)

1. **Repo-root only**:
   - Use a leading `/` for patterns that must only apply at the repository
     root.
   - Example: `/public/` ignores the root `public` directory only.
   - Non-root patterns (meant to match anywhere) should omit the leading
     slash, but only use that form when explicitly intended.

2. **Directory-only patterns must use a trailing slash**:
   - Always use a trailing `/` for directories.
   - Example: `/build/` (directory-only) vs `/build` (file or dir).
   - Keep file patterns without a trailing slash (e.g., `/package-lock.json`).

3. **Wildcard patterns**:
   - Keep wildcard rules (`*`, `**`, `?`) as-is unless converting to root
     anchored form is clearly intended by the rule owner.
   - If a wildcard is root-only, prefix it with `/` as well.

## Practical Steps When Editing

1. Normalize each entry to repo-root and dir/file form as described above.
2. Sort all lines alphabetically.
3. For each `!` override, move it directly after the line it overrides.
4. Re-scan for any pattern whose meaning changed unintentionally.
5. Leave a clean, minimal list with no duplicates.

## Examples

### Root-only, directory-only

- `/public/` ignores only the root `public` directory.
- `/build/` ignores only the root `build` directory.

### Root-only, file-only

- `/package-lock.json` ignores only the root lock file.

### Override pairing

```
/docs/source/ai-conversations/*
!/docs/source/ai-conversations/index.md
```

The override is intentionally adjacent, even if its `!` line would sort
elsewhere alphabetically.

## Do / Do Not

- Do keep rules concise and deterministic.
- Do preserve the intent of existing patterns when normalizing.
- Do not remove overrides or reorder them away from their base rule.
- Do not drop trailing slashes from directory patterns.
