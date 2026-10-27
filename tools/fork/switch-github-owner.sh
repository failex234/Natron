#!/usr/bin/env bash
# ***** BEGIN LICENSE BLOCK *****
# This file is part of Natron <https://natrongithub.github.io/>,
# (C) 2018-2026 The Natron developers
#
# Natron is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# Natron is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Natron.  If not, see <http://www.gnu.org/licenses/gpl-2.0.html>
# ***** END LICENSE BLOCK *****

# Point the clone/download URLs of a Natron checkout (and optionally of the
# plug-in checkouts) at forks owned by another GitHub account.
#
# Only URLs that are actually fetched are rewritten (submodules, build-script
# clone URLs, the OpenColorIO-Configs archive). Links to upstream issues,
# commits, pull requests and release assets are left alone on purpose, as are
# repositories that were not forked.
#
# Usage:
#   tools/fork/switch-github-owner.sh [--apply] [--owner NAME] [--from NAME] [--repo DIR]...
#
#   --apply       write the changes (default: dry run, only show the diff)
#   --owner NAME  new GitHub owner (default: failex234)
#   --from NAME   old GitHub owner (default: NatronGitHub)
#   --repo DIR    checkout to process; may be repeated
#                 (default: the Natron checkout containing this script)

set -euo pipefail

NEW_OWNER=failex234
OLD_OWNER=NatronGitHub
APPLY=0
REPOS=()

# Repositories that exist as forks under NEW_OWNER. Keep in sync with the forks.
FORKED="Natron openfx openfx-supportext SequenceParsing openfx-io openfx-misc OpenColorIO-Configs openfx-arena google-test google-mock google-breakpad"

# Files whose URLs are fetched. Paths are relative to the checkout; missing files are skipped.
FILES=".gitmodules tools/jenkins/gitRepositories.sh tools/jenkins/build-natron.sh tools/docker/natron-build/Dockerfile .github/workflows/ci.yml"

while [ $# -gt 0 ]; do
    case "$1" in
        --apply) APPLY=1 ;;
        --owner) NEW_OWNER="$2"; shift ;;
        --from) OLD_OWNER="$2"; shift ;;
        --repo) REPOS+=("$2"); shift ;;
        -h|--help) sed -n '20,35p' "$0"; exit 0 ;;
        *) echo "unknown argument: $1" >&2; exit 2 ;;
    esac
    shift
done

if [ ${#REPOS[@]} -eq 0 ]; then
    REPOS=("$(cd "$(dirname "$0")/../.." && pwd)")
fi

# "openfx" must not match "openfx-io": the repo name has to be followed by ".git",
# "/archive/", a quote, whitespace, "}" or the end of the line.
NAMES=$(echo "$FORKED" | tr ' ' '|')
export OLD_OWNER NEW_OWNER NAMES
REWRITE='s~github\.com/\Q$ENV{OLD_OWNER}\E/($ENV{NAMES})(?=\.git\b|/archive/|["'"'"'\s}]|$)~github.com/$ENV{NEW_OWNER}/$1~g'

changed=0
for repo in "${REPOS[@]}"; do
    if [ ! -d "$repo/.git" ] && [ ! -f "$repo/.git" ]; then
        echo "skipping $repo: not a git checkout" >&2
        continue
    fi
    echo "== $repo"
    for f in $FILES; do
        path="$repo/$f"
        [ -f "$path" ] || continue
        tmp=$(mktemp)
        perl -pe "$REWRITE" "$path" > "$tmp"
        if cmp -s "$path" "$tmp"; then
            rm -f "$tmp"
            continue
        fi
        diff -u --label "a/$f" --label "b/$f" "$path" "$tmp" || true
        changed=1
        if [ "$APPLY" -eq 1 ]; then
            cat "$tmp" > "$path" # keeps the file mode
        fi
        rm -f "$tmp"
    done
    if [ "$APPLY" -eq 1 ] && [ -f "$repo/.gitmodules" ]; then
        git -C "$repo" submodule sync --recursive >/dev/null
    fi
done

if [ "$changed" -eq 0 ]; then
    echo "nothing to change"
elif [ "$APPLY" -eq 0 ]; then
    echo
    echo "dry run: re-run with --apply to write these changes"
fi
