# NEON BREAKER – Cloud Build Kit

This kit is intended for a user who has an Android phone but no PC. GitHub Actions runs the build on GitHub's hosted Linux runner.

## What it does
- Downloads the Miyoo Mini/Plus ARM toolchain referenced by the upstream SDL2 port.
- Clones `steward-fu/sdl2`.
- Builds the Miyoo SDL2 runtime in the upstream Docker build environment.
- Cross-compiles `src/main.cpp` for ARM.
- Packages the test executable, SDL2/runtime libraries where found, and a launch script as a GitHub Actions artifact.

## Important
- This is an experimental build workflow. It has not yet been run successfully in this project and has not been tested on a Miyoo Mini Plus.
- The upstream SDL2 README states support for Miyoo Mini (Plus) and Stock OS / Onion v4.3.1-1. Compatibility with Red Onion 4.5.0 must be checked on-device.
- The generated program is only a smoke test: display, basic paddle/ball movement, input and a simple persistent best-score file. It is not the full NEON BREAKER port.
- The launch mechanism may need adapting to the exact Red Onion 4.5.0 package/launcher conventions. Do not overwrite system files or install over the original HTML game.
- If the workflow fails, open the failed Actions run and share the error log; the next iteration should fix the actual error, not guess.

## How to use from Android
1. Sign in to GitHub in your phone browser.
2. Create a new **public or private repository** named `neon-breaker-miyoo-test`.
3. In the repository, use **Add file → Upload files** and upload the contents of this kit (including the hidden `.github/workflows/build-miyoo.yml` path). Some mobile browser views hide dot-folders; if so, switch the browser to Desktop site.
4. Commit the uploaded files to the default branch.
5. Open the repository's **Actions** tab.
6. Select **Build NEON BREAKER for Miyoo Mini Plus**.
7. Press **Run workflow**.
8. When the run finishes, open it and download the artifact named `NEON_BREAKER_Miyoo_SDL2_Test_ARM`.

## If GitHub's mobile UI cannot upload the hidden workflow folder
Do not try to install the source ZIP on the Miyoo. Instead, tell me what you see on the GitHub page; we can use another route to create the workflow file.

## Upstream build reference
https://github.com/steward-fu/sdl2
