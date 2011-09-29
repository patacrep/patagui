#! /bin/bash
require_clean_work_tree () {
	git rev-parse --verify HEAD >/dev/null || exit 1
	git update-index -q --ignore-submodules --refresh
	err=0

	if ! git diff-files --quiet --ignore-submodules
	then
		echo >&2 "Cannot $1: You have unstaged changes."
		err=1
	fi

	if ! git diff-index --cached --quiet --ignore-submodules HEAD --
	then
		if [ $err = 0 ]
		then
		    echo >&2 "Cannot $1: Your index contains uncommitted changes."
		else
		    echo >&2 "Additionally, your index contains uncommitted changes."
		fi
		err=1
	fi

	if [ $err = 1 ]
	then
		test -n "$2" && echo >&2 "$2"
		exit 1
	fi
}


#require_clean_work_tree

rm -rf .build*
make
./macos_specific/makedmg.sh
echo " "
./macos_specific/sign -d Songbook-Client-$(git describe).dmg -v $(git describe) -p macos_specific/dsa_priv_songbookclient.pem -x > macos_specific/daily/$(date "+%Y-%m-%d")-$(git describe).feed
echo " "
echo "make sure too put this in feed.xml and upload it"

