#include <cstdlib>
#include <iostream>
#include <fstream>

#include <clang/Basic/TargetInfo.h>
#include <clang/Frontend/CompilerInstance.h>
#include <llvm/Support/Host.h>

void usage(char* name) {
   std::cerr << "USAGE: " << name << " filename.cpp line column" << std::endl;
   std::cerr << "Also list of files to consider must be in 'filelist'" << std::endl;
   exit(1);
}

int main(int argc, char** argv) {
   if(argc != 4) usage(argv[0]);
   std::string filename(argv[1]);
   size_t line = atoi(argv[2]);
   size_t column = atoi(argv[3]);
   std::ifstream filelist("filelist");
   if(!filelist) usage(argv[0]);

   clang::CompilerInstance ci;
   ci.createDiagnostics();
   llvm::IntrusiveRefCntPtr<clang::TargetOptions> opts(new clang::TargetOptions());
   opts->Triple = llvm::sys::getDefaultTargetTriple();
   clang::TargetInfo* info = clang::TargetInfo::CreateTargetInfo(ci.getDiagnostics(), opts.getPtr());
   ci.setTarget(info);

   ci.createFileManager();
   ci.createSourceManager(ci.getFileManager());
   ci.createPreprocessor();
   return 0;
}
