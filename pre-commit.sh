#!/bin/bash
# AUTOGENERATED COPYRIGHT HEADER START
# Copyright (C) 2024 Michael Fabian 'Xaymar' Dirks <info@xaymar.com>
# AUTOGENERATED COPYRIGHT HEADER END

_self=$(realpath "$0")
_self_dir=$(dirname "$_self")

if [ "$1" == "install" ]; then
	cp "${_self}" "${_self_dir}/.git/hooks/pre-commit"
	echo "Installed pre-commit hook to '${_self_dir}/.git/hooks/pre-commit'."
else
	declare -a FILES; readarray -t FILES < <(git diff --name-only --cached)
	node "$PWD/other/copyright.js" "${FILES[@]}"
fi
