
script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
project_dir="$script_dir/.."
cd $project_dir

if [ ! -f "$project_dir/build.bin" ]; then
	echo "info : removing build system."
	rm -fr $project_dir/build.bin.old
	echo "info : bootstraping the build system."
	gcc -std=gnu11 -Wall -Wextra -Werror -o $project_dir/build.bin $project_dir/build.c

	if [ $? -ne 0 ]; then
		echo "error: failed to bootstrap the build system."
		exit 1
	fi

	chmod +x $project_dir/build.bin
fi

exit 0
