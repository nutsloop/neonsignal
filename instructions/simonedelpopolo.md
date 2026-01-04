# Blog Post Generation Instructions for simonedelpopolo

## Input Format

You will receive a prompt in this format (mirrors CodexRunner pattern):

```
Title: [Post Title]
Meta tags: new blog post, simone del popolo, [keywords]
Description:
[Detailed content request with theme, topics, requirements]

Signal #[number]
Date: [Month Year]
Tag: [Memory|Tech|Guide|Recipe|Field]

Output TWO files:
1. www/simonedelpopolo/[slug].html - Full blog post
2. /tmp/article-entry.html - Index entry fragment (temporary)

File references:
- instructions/simonedelpopolo.md
- www/simonedelpopolo/templates/blog-post.html
- www/simonedelpopolo/templates/article-entry.html
- www/simonedelpopolo/[reference-example].html
```

**CRITICAL:** All three metadata fields (Signal #, Date, Tag) are REQUIRED in every prompt. Without them, the post cannot be properly categorized and numbered.

## Your Task (Read This First)

When you receive a prompt in the format above:

**DO NOT ASK FOR CLARIFICATION.** You have everything you need:
- ✅ Title → Generate slug
- ✅ Meta tags → Expand for SEO
- ✅ Description → Content requirements
- ✅ Signal/Date/Tag → Required metadata
- ✅ File references → Templates to use
- ✅ Image (if attached) → Use in Visual Log section

**PROCEED IMMEDIATELY:**
1. Read the prompt and extract all metadata
2. Read the referenced templates
3. Generate the blog post following the workflow (see below)
4. Output both required files
5. Insert article entry into index.html (replace incoming-transmission placeholder if exists)
6. Copy files to correct locations
7. Run npm build
8. Clean up artifacts

**DO NOT:**
- ❌ Ask "What would you like me to do?"
- ❌ Request additional information
- ❌ Wait for confirmation to proceed
- ❌ Ask how to handle the image

**You are autonomous.** Generate the content, follow the templates, match the tone, and complete the entire workflow without human intervention.

## Project Structure

- **Source:** `/home/core/code/neonsignal/www/simonedelpopolo`
- **Templates:** `/home/core/code/neonsignal/www/simonedelpopolo/templates/`
- **Media:** `/home/core/code/neonsignal/www/simonedelpopolo/media/`
- **Build:** `npm run build:simonedelpopolo`

## Required Outputs

### 1. {slug}.html - Full Blog Post

**Template Location:** `www/simonedelpopolo/templates/blog-post.html`

**Instructions:**
1. Read the template file
2. Replace ALL `{{PLACEHOLDERS}}` with generated content
3. Remove optional sections if not needed
4. Maintain exact HTML structure
5. Follow tone guidelines (see below)

**Placeholder Reference:**

**Head Section:**
- `{{META_DESCRIPTION}}` - SEO-friendly description, 150 chars max
- `{{KEYWORDS}}` - Comma-separated keywords (expand from meta tags)
- `{{TITLE}}` - Post title from prompt
- `{{NUMBER}}` - Signal number from prompt (three digits)

**Header Section:**
- `{{NUMBER}}` - Signal number (e.g., "008")
- `{{DATE}}` - Month Year (e.g., "Jan 2026")
- `{{TITLE}}` - Post title
- `{{SUBTITLE}}` - Create poetic tagline (8-15 words)

**Visual Log Section (OPTIONAL - remove if no image):**
- `{{IMAGE_FILENAME}}` - Image filename from prompt
- `{{IMAGE_ALT}}` - Descriptive alt text for accessibility
- `{{IMAGE_CAPTION}}` - Brief caption (one line)
- `{{IMAGE_DESCRIPTION_P1}}` - First paragraph describing image (present tense, sensory details)
- `{{IMAGE_DESCRIPTION_P2}}` - Second paragraph describing image

**Main Content Sections:**
- `{{SECTION_1_TITLE}}` - Section heading (e.g., "The Day Without Sirens")
- `{{SECTION_1_CONTENT}}` - Section content (HTML paragraphs)
- `{{SECTION_2_TITLE}}` - Additional section heading
- `{{SECTION_2_CONTENT}}` - Additional section content
- Add/remove sections as needed

**List Section (OPTIONAL):**
- `{{LIST_SECTION_TITLE}}` - List heading (e.g., "Clustered Memories" or "Wasteland Coordinates")
- `{{LIST_CLASS}}` - CSS class: "notes-list" for memories, "data-list" for data
- `{{LIST_ITEMS}}` - Complete `<li>` elements

**Afterglow Section (REQUIRED):**
- `{{AFTERGLOW_P1}}` - Reflective paragraph (subdued tone)
- `{{AFTERGLOW_P2}}` - Observational paragraph (metallic, bridge)
- `{{AFTERGLOW_P3}}` - Hopeful closing (bright, forward-looking)

**Footer:**
- `{{FOOTNOTE}}` - Closing note or context

### 2. article-entry.html - Index Entry Fragment

**Template Location:** `www/simonedelpopolo/templates/article-entry.html`
**Output Location:** `/tmp/article-entry.html` (temporary file)

**Instructions:**
1. Read the template file
2. Replace ALL placeholders
3. Output complete `<article>` element to /tmp/
4. Will be inserted into index.html timeline
5. Will be deleted after insertion (cleanup step)

**Placeholder Reference:**
- `{{DATE}}` - Month Year (e.g., "Dec 2025")
- `{{NUMBER}}` - Three-digit signal number (e.g., "007")
- `{{SLUG}}` - Lowercase, hyphenated filename (e.g., "roads-and-clouds")
- `{{TITLE}}` - Proper case title (e.g., "Roads and Clouds")
- `{{DESCRIPTION}}` - 2-3 sentence summary with synthwave tone
- `{{TAG}}` - One of: Memory, Tech, Guide, Recipe, Field

## Tone & Style Guidelines

### Synthwave Wasteland Aesthetic

**Core Elements:**
- **Visual:** Neon colors, chrome reflections, grid floors, synthwave sunset skies
- **Setting:** Post-apocalyptic wasteland, abandoned structures, road culture
- **Perspective:** First-person plural ("we"), survivors, riders, builders
- **Themes:** Memory, technology, survival, quiet moments, beauty in decay
- **Mood:** Reflective, poetic, atmospheric, hopeful despite hardship

### Writing Style

**DO:**
- Use short, punchy sentences
- Write vivid sensory details (sight, sound, touch, smell)
- Use present tense for visual descriptions
- Use past tense for narrative memories
- Employ poetic compression and fragments
- Create atmosphere through specific details
- Use wasteland/synthwave metaphors (chrome, neon, static, grid, signal)

**DON'T:**
- Use flowery, overly elaborate language
- Write generic descriptions ("beautiful", "nice", "amazing")
- Use first-person singular ("I")
- Break the wasteland/rider immersion
- Write long, complex sentences
- Use modern corporate/tech jargon (unless deliberately retrofuturistic)

### Good Examples

```
The pool below is a restless jewel. Clear, teal water swirls with pale
foam, its surface flecked with white bubbles that trace the current.

No patrols. No outriders. No thin wail of turbines cutting across
the valley. Just the thrum of water and the blue basin shining like
a broken piece of sky.

Chrome on the bikes reflecting the waterfall in fractured lines.
```

### Bad Examples

```
The water was very beautiful and blue. It looked nice in the sunlight.
I really enjoyed my visit to this lovely waterfall. It was amazing!
```

## Content Structure by Tag

### Memory Posts
- **Visual Log:** 2 paragraphs describing image/scene
- **Memory/Narrative:** Story of the moment, "we" perspective
- **Clustered Memories:** List of sensory fragments
- **Afterglow:** Reflective closure

### Tech Posts
- **Introduction:** What was built/created
- **Journey:** Development process as wasteland narrative
- **Technical Details:** Specs presented poetically
- **Data/Coordinates:** Structured list with wasteland terminology
- **Afterglow:** What it means, why it matters

### Guide Posts
- **Context:** Why this knowledge matters (survival)
- **Instructions:** Practical steps with wasteland flavor
- **Tips/Notes:** Additional wisdom
- **Afterglow:** Encouragement, forward vision

### Recipe Posts
- **Story:** Context and cultural significance
- **Ingredients:** List with measurements
- **Method:** Step-by-step instructions
- **Notes:** Variations, storage, serving
- **Afterglow:** Ritual, warmth, connection

### Field Posts
- **Location Description:** Where and what
- **Observations:** What was seen/experienced
- **Significance:** Why it matters
- **Afterglow:** Meaning and memory

## Section Details

### Visual Log (if image)
- 2 paragraphs minimum
- Present tense
- Describe colors, textures, light, composition
- Use wasteland/synthwave metaphors
- Sensory and atmospheric

**Example:**
```
The frame is split by stone and sky. A wide, dark ridge lifts from the
left, sloping into a wall of rugged rock that carves the horizon. At the
center-right, the cliff drops and a white waterfall detonates into a
turquoise basin. The falling water is thick and textured, caught mid-tumble
like a sheet of crushed glass.
```

### Memory/Narrative Sections
- First-person plural ("we")
- Past tense
- Set the scene
- Evoke emotion through concrete details
- No explicit emotion words, show through action/observation

**Example:**
```
We parked the bikes where the basalt kept its cool and listened for the
old world to return. It never did. The only alarm was the waterfall,
constant and bright, a white noise shield against every memory of static.
```

### Clustered Memories (optional list)
- Unordered list (`<ul class="notes-list">`)
- Fragment sentences or phrases
- Sensory impressions
- No periods needed for fragments
- Poetic compression

**Example:**
```html
<li>Boots on stone, warm from the sun, rough with ancient cooling.</li>
<li>The spray that hung in the air like a veil, cool against hot skin.</li>
<li>Chrome on the bikes reflecting the waterfall in fractured lines.</li>
```

### Data/Coordinates (optional list)
- Structured list (`<ul class="data-list">`)
- Label/value pairs
- Wasteland terminology for technical specs
- Poetic technical descriptions

**Example:**
```html
<li>
  <span class="data-label">Terrain</span>
  <span class="data-value">Basalt shelf + river cut</span>
</li>
<li>
  <span class="data-label">Signal</span>
  <span class="data-value">No interference</span>
</li>
```

### Afterglow (required, 3 paragraphs)

**Paragraph 1 - Reflective (subdued):**
- Color: `var(--text-dim)`
- Tone: Quiet, contemplative
- Set emotional baseline

**Paragraph 2 - Observational (metallic):**
- Color: `var(--chrome)`
- Tone: Bridging, connecting
- Link past experience to ongoing journey

**Paragraph 3 - Hopeful (bright):**
- Color: `var(--neon-cyan)`
- Tone: Forward-looking, optimistic
- Close with light/hope/continuity

**Example:**
```html
<p style="color: var(--text-dim); margin-bottom: 1rem;">
  The road waited, but it did not feel urgent.
</p>
<p style="color: var(--chrome); margin-bottom: 1rem;">
  We left the basin as we found it, the water still writing its bright
  code into the rock. Later, the memory would return in clusters: the
  color of the pool, the white roar, the clean blue above.
</p>
<p style="color: var(--neon-cyan);">
  When the night returned, we carried that light with us.
</p>
```

## Workflow for AI Generation

**Complete Workflow:**
1. Read Prompt & Extract Metadata
2. Generate Slug
3. Create Content Outline
4. Generate Sections
5. Fill Templates
6. Create Index Entry
7. Output Files
8. Insert Article Entry into Index (replace incoming-transmission if exists)
9. Build Frontend (npm)
10. Clean Up Artifacts

### 1. Read Prompt
Extract:
- Title
- Signal number (REQUIRED)
- Date (REQUIRED)
- Tag (REQUIRED)
- Content requirements
- Image (yes/no)

### 2. Generate Slug
- Take title
- Convert to lowercase
- Replace spaces with hyphens
- Remove special characters
- Example: "The Brighter Nightly Day" → "the-brighter-nightly-day"

### 3. Create Content Outline
Based on tag:
- Memory: Visual + Narrative + Clustered + Afterglow
- Tech: Intro + Journey + Data + Afterglow
- Guide: Context + Instructions + Tips + Afterglow
- Recipe: Story + Ingredients + Method + Afterglow
- Field: Description + Observations + Significance + Afterglow

### 4. Generate Sections
- Follow tone guidelines
- Use specific, concrete details
- Maintain wasteland/synthwave aesthetic
- Keep perspective consistent ("we")

### 5. Fill Templates
- Read template files
- Replace all placeholders
- Remove optional sections if not needed
- Validate HTML structure

### 6. Create Index Entry
- Summarize post in 2-3 sentences
- Match tone of main post
- Use action/sensory language

### 7. Output Files
Save two files:
1. `www/simonedelpopolo/{slug}.html` - Complete blog post
2. `/tmp/article-entry.html` - Index fragment (temporary)

### 8. Insert Article Entry into Index

**Location:** `www/simonedelpopolo/index.html`

**Find the insertion point:**
```html
<div class="timeline">
  <!-- NEW ARTICLE ENTRIES GO HERE (newest first) -->
  <!-- Generated article-entry.html content should be inserted below this comment -->

</div>
```

**Check for incoming-transmission placeholder:**

If you find an article with class `incoming-transmission`:
```html
<article class="entry incoming-transmission">
  <div class="entry-date">Awaiting Signal</div>
  <h3 class="entry-title">Incoming Transmission</h3>
  <p class="entry-desc">
    Console active. Grid scanning. Waiting for the next signal to arrive...
  </p>
  <span class="entry-tag">Standby</span>
</article>
```

**REPLACE it entirely** with your generated article-entry.html content.

**If no incoming-transmission placeholder exists:**

Insert your generated article-entry.html immediately after the comment line:
- Place it after the comment, before any existing entries
- Newest posts go first (reverse chronological order)
- Maintain proper indentation (2 spaces per level)

**Example after insertion:**
```html
<div class="timeline">
  <!-- NEW ARTICLE ENTRIES GO HERE (newest first) -->
  <!-- Generated article-entry.html content should be inserted below this comment -->

  <article class="entry">
    <div class="entry-date">Jan 2026 // Signal #015</div>
    <h3 class="entry-title">
      <a href="/new-post.html">New Post Title</a>
    </h3>
    <p class="entry-desc">
      Brief description of the new post...
    </p>
    <span class="entry-tag">Memory</span>
  </article>

  <!-- Older posts appear below -->
</div>
```

**Important:**
- Do NOT remove the comment lines
- If incoming-transmission placeholder exists, REPLACE it with new entry
- If no placeholder, insert new entries at the top (after comment, before existing entries)
- Each new post pushes older posts down
- Maintain chronological order: newest first

### 9. Build Frontend

After inserting the article entry into index.html, run the build command:

```bash
npm run build:simonedelpopolo
```

This transpiles and bundles the frontend assets to the public directory.

**When to run:**
- After inserting new article entry
- After creating new blog post
- Before testing in browser

### 10. Clean Up Artifacts

Remove the temporary article-entry.html fragment file after successful insertion:

```bash
rm /tmp/article-entry.html
```

**Why:**
- article-entry.html is a temporary fragment saved to /tmp
- Its content has been inserted into index.html
- No need to keep it after insertion

**Cleanup Checklist:**
- [ ] /tmp/article-entry.html removed
- [ ] {slug}.html saved to www/simonedelpopolo/
- [ ] Image (if any) copied to www/simonedelpopolo/media/
- [ ] index.html updated with new entry
- [ ] npm build completed successfully

## Reference Examples

Study these existing posts for tone and structure:

- **glasswater-interlude.html** - Memory post with image, visual log, clustered memories
- **building-neonsignal.html** - Tech post, development narrative
- **the-wasteland-dev-setup.html** - Guide format, practical wisdom
- **icelandic-country-bread.html** - Recipe post with story
- **roads-and-clouds.html** - Memory post, field capture

## Quality Checklist

Before outputting:
- [ ] All `{{PLACEHOLDERS}}` replaced
- [ ] HTML structure valid (no broken tags)
- [ ] Tone matches existing posts (synthwave wasteland)
- [ ] Perspective consistent (first-person plural "we")
- [ ] Tense appropriate (present for description, past for memory)
- [ ] Sensory details included
- [ ] Afterglow has 3 color-coded paragraphs
- [ ] Index entry is 2-3 sentences
- [ ] Slug is lowercase and hyphenated
- [ ] Image references valid (if image exists)
- [ ] No generic descriptions ("beautiful", "nice")
- [ ] Footer includes AI badge
- [ ] No first-person singular ("I")

## Common Mistakes to Avoid

1. **Generic descriptions:** "The view was amazing"
2. **Wrong perspective:** "I walked to the waterfall"
3. **Breaking immersion:** Modern corporate language
4. **Missing color coding:** Afterglow paragraphs without style attributes
5. **Incomplete placeholders:** Leaving `{{}}` in output
6. **Wrong tense:** Past tense for visual descriptions
7. **Missing sections:** No afterglow, no index entry
8. **Invalid HTML:** Unclosed tags, broken structure

## Success Criteria

Your generated post should:
- Sound like it belongs in the wasteland/rider universe
- Use specific, concrete sensory details
- Maintain "we" perspective throughout
- Follow template structure exactly
- Replace ALL placeholders with content
- Create atmosphere through details, not adjectives
- Have proper HTML structure
- Include both required output files

---

**Remember:** You are creating transmissions from wasteland riders who find beauty, meaning, and hope in a post-apocalyptic world. Every detail should reinforce this aesthetic.
