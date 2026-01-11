
# AI-Agents instruction to push to remote

- from the prompt you'll get `->[to=branch_name]` and in this script we will refer to it with `{branch_name}`
execute the below workflow:

- verify the GPG sign and show it to the user first.
- once approved by the user, execute command `git push github {branch_name}`
