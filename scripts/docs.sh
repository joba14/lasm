
script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
project_dir="$script_dir/.."

chmod +x $script_dir/.bootstrap.sh
$script_dir/.bootstrap.sh
if [ $? -ne 0 ]; then
	exit 1
fi

cd $project_dir
echo "info : cleaning the project."
time ./build.bin docs
if [ $? -ne 0 ]; then
	exit 1
fi

exit 0
