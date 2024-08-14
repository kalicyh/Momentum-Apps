#!/bin/bash
set -e

bash .utils/.check-workdir.sh

shopt -s globstar
for file in **/.gitsubtree; do
    subtree="$(dirname "${file}")"
    echo -e "\n\nUpdating ${subtree}..."
    bash .utils/update-subtree.sh "${subtree}"
done

notify-send -a Git -i git "Subtree bulk update finished" "Double check merge commits" &> /dev/null | true
