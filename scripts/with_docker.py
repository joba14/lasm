
import subprocess
import argparse
import os


def image_exists(image_name: str) -> bool:
	result: subprocess.CompletedProcess[str] = subprocess.run(
		[f'docker', f'images', f'--format', '{{.Repository}}:{{.Tag}}'], capture_output=True, text=True
	)
	images: list[str] = result.stdout.splitlines()
	return image_name in images


def main() -> None:
	parser: argparse.ArgumentParser = argparse.ArgumentParser(description=f'Join multiple command-line arguments with spaces.')
	parser.add_argument(f'cmd_args', nargs=f'+', help=f'List of command-line arguments to join')
	args: argparse.Namespace = parser.parse_args()

	script_path: str = os.getcwd()
	project_dir: str = os.path.dirname(script_path)
	os.chdir(project_dir)


	image_name: str = f'lasm_development_container_image:0.1'
	if not image_exists(image_name):
		subprocess.run(
			[f'docker', f'build', f'-f', f'./.dockerfile', f'-t', image_name, f'.']
		)

	command: str = f'cd ./scripts && python3 ./build.py ' + f' '.join(args.cmd_args)
	subprocess.run(
		[f'docker', f'run', f'--rm', f'--name', f'lasm_development_container', f'-v', f'{project_dir}:/workspace', f'-w', f'/workspace', f'lasm_development_container_image:0.1', f'/bin/bash', f'-c', command]
	)
	exit(0)


if __name__ == f'__main__':
	main()
