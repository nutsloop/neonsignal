# AI-Agents instruction to commit !only! staged files

execute the below workflow:

- run the command -> `git status --long --verbose`
- show to the user what staged.
- always append the commit message with:
  - Co-Authored-By: OpenAI Codex <noreply@openai.com>
  - Co-Authored-By: Anthropic Claude <noreply@anthropic.com>
- show a preview of the commit message, following CLAUDE.md & AGENTS.md guidelines.
- once approved, by the user, commit the staged files.
