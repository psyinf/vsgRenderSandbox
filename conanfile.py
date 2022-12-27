from conans import ConanFile
from conan.tools.cmake import CMake

class VsgSandbox(ConanFile):
    name = "VsgSandbox"
    version = "0.1"
    settings = "os", "compiler", "build_type", "arch"
    requires = [("vsg/1.0.0")
                ]
    generators = "cmake_find_package_multi"
    
    def configure(self):
        self.options['vsg'].shared = False 
    def imports(self):    
        self.copy("*.dll", "bin", "bin")
        
