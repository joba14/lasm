
if [ $# -ne 1 ]; then
	echo "error: lint configuration was not provided"
	echo "usage: $0 <debug|release>"
	exit 1
fi

build_configuration=$1
if [ "$build_configuration" != "debug" ] && [ "$build_configuration" != "release" ]; then
	echo "error: argument must be either 'debug' or 'release'"
	exit 1
fi

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
project_dir="$script_dir/.."

chmod +x $script_dir/.bootstrap.sh
$script_dir/.bootstrap.sh
if [ $? -ne 0 ]; then
	exit 1
fi

cd $project_dir
echo "info : building the project."
time ./build.bin lint_$build_configuration
if [ $? -ne 0 ]; then
	exit 1
fi

exit 0
