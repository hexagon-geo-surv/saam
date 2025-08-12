<!--
SPDX-FileCopyrightText: Leica Geosystems AG

SPDX-License-Identifier: MIT
-->

# Contributing to saam

## Restricted upstream repo
In order to keep the upstream repo clean, the following is expected from contributors.

All contributions are expected from forks. No development/bugfixing branches,
private commits/tags are expected in the upstream repo. This way the upstream
repo is reserved only for the stable versions. It may only contain discussed and agreed solutions.

Applying this strict policy, authors expect to maintain a clean, easily searchable history,
where the history of the development is not "lost" because of a branching hell.

## Private forks
All development is expected to be done in private forks.
If you want to have your contribution tested by the CI, please open a PR or a draft PR.

All your improvement ideas, experiments, prototypes can stay in your private fork - you will not lose them.
You can come back to them anytime. Your fork is your only yours, it is up to you how clean you keep it.

## Signed commits, Sign-off-by
Commits are expected to be cryptographically signed.

Contributors are required to make sure that their contribution is legit. 
Therefore commits needs to be signed-off.

The recommended git commit command line:
``` bash
> git commit -s -S -m "Your commit message"
```