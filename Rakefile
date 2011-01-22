
# clobber / clean tasks

require 'rake/clean'

EXT_CONF = 'ext/extconf.rb'
MAKEFILE = 'ext/Makefile'
MODULE = 'ext/rlibemu.so'
SRC = Dir.glob('ext/*.c')
SRC << MAKEFILE

CLEAN.include [ 'ext/*.o', 'ext/depend', MODULE ]
CLOBBER.include [ 'config.save', 'ext/mkmf.log', 'ext/extconf.h', MAKEFILE ]

file MAKEFILE => EXT_CONF do |t|
	Dir::chdir(File::dirname(EXT_CONF)) do
		unless sh "ruby #{File::basename(EXT_CONF)}"
			$stderr.puts "Failed to run extconf"
			break
		end
	end
end

file MODULE => SRC do |t|
	Dir::chdir(File::dirname(EXT_CONF)) do
		unless sh "make"
			$stderr.puts "make failed"
			break
		end
	end
end

desc "Build the native library"
task :build => MODULE

# gem task

require 'rake/gempackagetask'

PKG_FILES = FileList[
	"Rakefile", "NEWS", "README.rdoc", 
	"ext/*.[ch]", "ext/extconf.rb" 
	]

SPEC = Gem::Specification.new do |s|
	s.name = "ruby-libemu"
	s.version = "0.0.1"
	s.email = "juan.vazquez.test@gmail.com"
	s.homepage = "http://www.testpurposes.net"
	s.summary = "Ruby binding for libemu"
	s.files = PKG_FILES
	s.required_ruby_version = '>= 1.8.7'
	s.extensions = "ext/extconf.rb"
	s.author = "juan vazquez"
	#s.rubyforge_project = ""
	s.description = "Ruby binding to the libemu x86 emulator."
end

Rake::GemPackageTask.new(SPEC) do |pkg|
	pkg.need_tar = true
	pkg.need_zip = true
end

# rdoc / ri tasks

require 'rake/rdoctask'

RDOC_FILES = FileList["README.rdoc", "ext/rlibemu.c"]

Rake::RDocTask.new do |rd|
	rd.main = "README.rdoc"
	rd.rdoc_dir = "doc/site/api"
	rd.rdoc_files.include(RDOC_FILES)
end

Rake::RDocTask.new(:ri) do |rd|
	rd.main = "README.rdoc"
	rd.rdoc_dir = "doc/ri"
	rd.options << "--ri-system"
	rd.rdoc_files.include(RDOC_FILES)
end
