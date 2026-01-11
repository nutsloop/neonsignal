# AI-Agents instruction to commit !only! staged files

execute the below workflow:

- run the command -> `git status --long --verbose`
- show to the user what staged.
- always append the commit message with:
  - AI-Agents used at 99% 
- avoid backticks in the `git commit -m` message because the shell treats them as command substitution; use plain text or single quotes instead (or `$'...'` for newlines).
- show a preview of the commit message, following CLAUDE.md & AGENTS.md guidelines.
- once approved, by the user, commit the staged files.
