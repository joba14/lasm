
import subprocess
import argparse
import os


def image_exists(image_name: str) -> bool:
	result: subprocess.CompletedProcess[str] = subprocess.run(
		['docker', 'images', '--format', '{{.Repository}}:{{.Tag}}'],
		capture_output=True,
		text=True
	)
	images: list[str] = result.stdout.splitlines()
	return image_name in images


def main() -> None:
	parser: argparse.ArgumentParser = argparse.ArgumentParser()
	subparsers: argparse._SubParsersAction = parser.add_subparsers(dest='command')
	parser_clean = subparsers.add_parser('clean', help='Clean the project')
	parser_clean.add_argument('args', nargs='*', help='Arguments for clean command')
	parser_build = subparsers.add_parser('build', help='Build the project')
	parser_build.add_argument('args', nargs='*', help='Arguments for build command')
	parser_docs = subparsers.add_parser('docs', help='Generate documentation')
	parser_docs.add_argument('args', nargs='*', help='Arguments for docs command')
	parser_lint = subparsers.add_parser('lint', help='Lint the project')
	parser_lint.add_argument('args', nargs='*', help='Arguments for lint command')
	args = parser.parse_args()

	script_path: str = os.getcwd()
	project_dir: str = os.path.dirname(script_path)
	os.chdir(project_dir)

	concat_args: str = ' '.join(args.args)

	if args.command == 'clean':
		command: str = f'cd ./scripts && chmod +x ./clean.sh && ./clean.sh {concat_args}'
	elif args.command == 'build':
		command: str = f'cd ./scripts && chmod +x ./build.sh && ./build.sh {concat_args}'
	elif args.command == 'docs':
		command: str = f'cd ./scripts && chmod +x ./docs.sh && ./docs.sh {concat_args}'
	elif args.command == 'lint':
		command: str = f'cd ./scripts && chmod +x ./lint.sh && ./lint.sh {concat_args}'
	else:
		assert False, "invalid command found!"

	image_name: str = 'lasm_development_container_image:0.1'
	if not image_exists(image_name):
		subprocess.run(['docker', 'build', '-f', './.dockerfile', '-t', image_name, '.'])
	subprocess.run(['docker', 'run', '--rm', '--name', 'lasm_development_container', '-v', f'{project_dir}:/workspace', '-w', '/workspace', 'lasm_development_container_image:0.1', '/bin/bash', '-c', command])
	exit(0)


if __name__ == '__main__':
	main()
