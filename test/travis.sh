#!/usr/bin/env /bin/bash

source "$(dirname "$0")/common.sh"

msg "Testing $TRAVIS_BRANCH"
echo


if [[ "$TRAVIS_BRANCH" == "master" || "$TRAVIS_PULL_REQUEST" = "false" ]]; then
	die "No tests are to be run on master and non-pull requests -- have a nice day!";
	exit 1;
fi

case "$TRAVIS_BRANCH" in
	*) cd "$TRAVIS_BRANCH" || die "Branch '$TRAVIS_BRANCH' doesn't match any assignment";;
esac

if [[ -r test.sh ]] ; then
	bash test.sh
elif [[ -r Makefile ]] ; then
	make test
else
	die "Don't know how to test '$TRAVIS_BRANCH'"
fi

# vim: set sw=4 sts=4 ts=4 noet :
