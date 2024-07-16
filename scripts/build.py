#!/usr/bin/env python3

import subprocess
import argparse
import sys
import os


configurations: list[str] = [f'release', f'debug']
build_c_name: str = f'build.c'
build_bin_name: str = f'build.bin'
build_bin_old_name: str = f'build.bin.old'


def bootstrap_build_system(project_dir: str) -> bool:
	build_bin_path: str = os.path.join(project_dir, build_bin_name)

	if not os.path.isfile(build_bin_path):
		print(f'info : removing build system.')
		old_build_bin_path: str = os.path.join(project_dir, build_bin_old_name)
		if os.path.exists(old_build_bin_path):
			os.remove(old_build_bin_path)
		
		print(f'info : bootstrapping the build system.')
		result: subprocess.CompletedProcess[bytes] = subprocess.run(
			[f'gcc', f'-std=gnu11', f'-Wall', f'-Wextra', f'-Werror', f'-o', build_bin_path, os.path.join(project_dir, build_c_name)]
		)
		if result.returncode != 0:
			print(f'error: failed to bootstrap the build system.')
			return False

		os.chmod(build_bin_path, 0o755)
	return True


def clean_project(project_dir: str) -> bool:
	if not bootstrap_build_system(project_dir):
		return False

	print(f'info : cleaning the project.')
	result: subprocess.CompletedProcess[bytes] = subprocess.run(
		[os.path.join(project_dir, build_bin_name), f'clean'], cwd=project_dir
	)
	if result.returncode != 0:
		return False

	return True


def build_project(build_configuration: str, project_dir: str) -> bool:
	if build_configuration not in configurations:
		print(f'error: build configuration must be one of the {configurations}')
		return False

	if not bootstrap_build_system(project_dir):
		return False

	print(f'info : building the project.')
	result: subprocess.CompletedProcess[bytes] = subprocess.run(
		[os.path.join(project_dir, build_bin_name), f'build_{build_configuration}'], cwd=project_dir
	)
	if result.returncode != 0:
		return False

	return True


def generate_docs(project_dir: str) -> bool:
	if not bootstrap_build_system(project_dir):
		return False

	print(f'info : generating documentation.')
	result: subprocess.CompletedProcess[bytes] = subprocess.run(
		[os.path.join(project_dir, build_bin_name), f'docs'], cwd=project_dir
	)
	if result.returncode != 0:
		return False

	return True


def lint_project(build_configuration: str, project_dir: str) -> bool:
	if build_configuration not in configurations:
		print(f'error: build configuration must be one of the {configurations}')
		return False

	if not bootstrap_build_system(project_dir):
		return False

	print(f'info : linting the project for {build_configuration}.')
	result: subprocess.CompletedProcess[bytes] = subprocess.run(
		[os.path.join(project_dir, build_bin_name), f'lint_{build_configuration}'], cwd=project_dir
	)
	if result.returncode != 0:
		return False

	return True


def main() -> None:
	parser: argparse.ArgumentParser = argparse.ArgumentParser()
	subparsers: argparse._SubParsersAction[argparse.ArgumentParser] = parser.add_subparsers(dest=f'command', required=True, help=f'Command to execute')
	clean_parser: argparse.ArgumentParser = subparsers.add_parser(f'clean', help=f'The clean command')
	build_parser: argparse.ArgumentParser = subparsers.add_parser(f'build', help=f'The build command')
	build_parser.add_argument(f'--config', choices=configurations, type=str, default=configurations[0], help=f'Build configuration')
	docs_parser:  argparse.ArgumentParser = subparsers.add_parser(f'docs', help=f'The docs command')
	lint_parser:  argparse.ArgumentParser = subparsers.add_parser(f'lint', help=f'The lint command')
	lint_parser.add_argument(f'--config', choices=configurations, type=str, default=configurations[0], help=f'Lint configuration')
	args: argparse.Namespace = parser.parse_args()

	script_path: str = os.getcwd()
	project_dir: str = os.path.dirname(script_path)
	os.chdir(project_dir)

	if args.command == f'clean':
		if not clean_project(project_dir):
			sys.exit(1)
	elif args.command == f'build':
		if not build_project(args.config, project_dir):
			sys.exit(1)
	elif args.command == f'docs':
		if not generate_docs(project_dir):
			sys.exit(1)
	elif args.command == f'lint':
		if not lint_project(args.config, project_dir):
			sys.exit(1)
	else:
		print(f'error: invalid command \'{args.command}\'')
		parser.print_help()
		sys.exit(1)


if __name__ == f'__main__':
	main()
