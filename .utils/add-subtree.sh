#!/bin/bash
set -e

if [ "$(git rev-parse --show-prefix)" != "" ]; then
    echo "Must be in root of git repo!"
    exit
fi
if ! git diff --quiet || ! git diff --cached --quiet || ! git merge HEAD &> /dev/null; then
    echo "Workdir must be clean!"
    exit
fi

if [ "$1" = "" ] || [ "$2" = "" ]; then
    echo "Usage 1: <path> <repo url> <branch> [subdir]"
    echo "Usage 2: <path> <repo url>/tree/<branch>[/subdir]"
    exit
fi
path="${1%/}"
repo="${2%/}"
if [ "$3" = "" ]; then
    read repo branch subdir <<< "$(sed -E "s|(https?://[^/]+)/([^/]+)/([^/]+)/(tree\|blob)/([^/]+)/?(.*)|\1/\2/\3 \5 \6|" <<< "${repo}")"
else
    branch="${3}"
    subdir="${4%/}"
fi
gitsubtree="${path}/.gitsubtree"

prevremotedir=""
if [ -e "${gitsubtree}" ]; then
    echo "Subtree already exists, adding new remote to it."
    prevremotedir="$(mktemp -d /tmp/gitsubtree-XXXXXXXX)"
    mv -T "${path}" "${prevremotedir}"
    git add "${path}"
    git commit -m "Add new remote for ${path}"
fi

if [ "${subdir}" = "" ]; then
    subdir="/"
    git subtree add -P "${path}" "${repo}" "${branch}" -m "Add ${path} from ${repo}"
else
    bash .utils/.subtree-subdir-helper.sh "${path}" "${repo}" "${branch}" "${subdir}" add
fi

if [ "${prevremotedir}" != "" ]; then
    rm -r "${path}"
    mv -T "${prevremotedir}" "${path}"
fi

echo "${repo} ${branch} ${subdir}" >> "${gitsubtree}"
git add "${gitsubtree}"
git commit --amend --no-edit

if [ "${prevremotedir}" != "" ]; then
    prevremotedir=""
    echo "Added new remote for existing subtree, you must solve conflicts manually..."
fi
