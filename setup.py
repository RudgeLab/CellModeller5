import os
import sys
import platform
import subprocess

import setuptools
from setuptools import setup, find_packages, Extension
from setuptools.command.develop import develop
from setuptools.command.build_ext import build_ext

from distutils.version import LooseVersion
from distutils import log

class DevelopDebugPackage(develop):
	def install_for_development(self):
		self.run_command('egg_info')

		# Build extensions in-place
		command = self.reinitialize_command('build_ext', inplace=1)
		command.debug = True

		self.run_command('build_ext')

		if setuptools.bootstrap_install_from:
			self.easy_install(setuptools.bootstrap_install_from)
			setuptools.bootstrap_install_from = None

		self.install_namespaces()

		# create an .egg-link in the installation dir, pointing to our egg
		log.info("Creating %s (link to %s)", self.egg_link, self.egg_base)
		if not self.dry_run:
			with open(self.egg_link, "w") as f:
				f.write(self.egg_path + "\n" + self.setup_path)
		# postprocess the installed distro, fixing up .pth, installing scripts,
		# and handling requirements
		self.process_distribution(None, self.dist, not self.no_deps)

class CMakeExtension(Extension):
	def __init__(self, name, sourcedir=''):
		Extension.__init__(self, name, sources=[])
		self.sourcedir = os.path.abspath(sourcedir)

class CMakeBuild(build_ext):
	def run(self):
		for ext in self.extensions:
			self.build_extension(ext)

	def build_extension(self, ext):
		extpath = self.get_ext_fullpath(ext.name)
		extdir = os.path.abspath(os.path.dirname(extpath))

		extnametokens = os.path.basename(extpath).split(".")
		extname = ".".join(extnametokens[0:-1])
		extsuffix = "." + extnametokens[-1]

		cfg = "Debug" if self.debug else "Release"

		cmake_args = [f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}",
					  f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{cfg.upper()}={extdir}",
					  f"-DCMAKE_BUILD_TYPE={cfg}",
					  f"-DPYTHON_EXECUTABLE={sys.executable}",
					  f"-DCELLMODELLER_ARTIFACT_NAME={extname}",
					  f"-DCELLMODELLER_ARTIFACT_SUFFIX={extsuffix}",
		]
		build_args = [ "--config", cfg ]

		# Note: I don't think this is just a Windows-specific option
		if platform.system() == "Windows" and sys.maxsize > 2**32:
			cmake_args += [ "-A", "x64" ]

		env = os.environ.copy()
#		env["CXXFLAGS"] = '{} -DVERSION_INFO=\\"{}\\"'.format(env.get('CXXFLAGS', ''),
#								self.distribution.get_version())

		if not os.path.exists(self.build_temp):
			os.makedirs(self.build_temp)

		subprocess.check_call([ "cmake", ext.sourcedir ] + cmake_args, cwd=self.build_temp, env=env)
		subprocess.check_call([ "cmake", "--build", "." ] + build_args, cwd=self.build_temp)

setup(
	name="cellmodeller5",
	version="1.0",
	packages=[ "cellmodeller5", "cellmodeller5.shaders" ],
	package_dir={ "cellmodeller5": "cellmodeller5", },
	package_data={
		"cellmodeller5": ["shaders/*.glsl"],
	},
	include_package_data=True,
	install_requires=[ "numpy" ],
	ext_modules=[ CMakeExtension("cellmodeller5.native") ],
	cmdclass={
		"build_ext": CMakeBuild,
		"develop_debug": DevelopDebugPackage,
	},
	zip_safe=True,
)