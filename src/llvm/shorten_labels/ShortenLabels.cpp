#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/IR/Module.h"

#include <functional>
#include <sstream>

using namespace llvm;

namespace {
  struct ShortenLabels : public ModulePass {
    static char ID;
    ShortenLabels() : ModulePass(ID) {}

    bool runOnBasicBlock(BasicBlock &bb) {
        bool changed=false;

        auto *symbols=bb.getValueSymbolTable();
        auto it=symbols->begin();
        while(it!=symbols->end()){
           //errs() << "   "<<it->getValue()->getName().str()<<"\n";
            std::string name=it->getValue()->getName().str();
            if(name.size()>64){
                std::hash<std::string> hasher;
                auto hash=hasher(name);
                std::stringstream acc;
                acc<<name.substr(0,48)<<"_"<<hash;

                std::string newName=acc.str();

                //errs() <<"      renaming "<<name<<"\n";
                //errs() <<"      to "<<newName<<"\n";

                it->getValue()->setName(newName);

                changed=true;
                it=symbols->begin();
            }else{
                ++it;
            }
        }
        return changed;
    }

    bool runOnFunction(Function &F) {
        bool changed=false;

        //errs() << "Function: ";
        //errs().write_escaped(F.getName()) << '\n';
        std::string name=F.getName().str();

        if(name.size()>64){
            std::hash<std::string> hasher;
            auto hash=hasher(name);
            std::stringstream acc;
            acc<<name.substr(0,48)<<"_"<<hash;

            std::string newName=acc.str();

            //errs() <<"  renaming "<<name<<"\n";
            //errs() <<"  to "<<newName<<"\n";

            F.setName(newName);

            changed=true;
        }

        while(1){
            bool out=true;
            for(auto &bb : F){
                if(runOnBasicBlock(bb)){
                    out=false;
                    changed=true;
                }
            }
            if(out)
                break;
        }

      return changed;
    }

    bool runOnModule(Module &m)
    {
        bool changed=false;

        auto &flist=m.getFunctionList();
        for(auto &F : flist){
            runOnFunction(F);
        }


        auto &symbols=m.getValueSymbolTable();
        auto it=symbols.begin();
        while(it!=symbols.end()){
            //errs() << "   "<<it->getValue()->getName().str()<<"\n";
            std::string name=it->getValue()->getName().str();
            if(name.size()>64){
                std::hash<std::string> hasher;
                auto hash=hasher(name);
                std::stringstream acc;
                acc<<name.substr(0,48)<<"_"<<hash;

                std::string newName=acc.str();

                //errs() <<"      renaming "<<name<<"\n";
                //errs() <<"      to "<<newName<<"\n";

                it->getValue()->setName(newName);

                changed=true;
                it=symbols.begin();
            }else{
                ++it;
            }
        }
        return changed;
    }
  };
}

char ShortenLabels::ID = 0;
static RegisterPass<ShortenLabels> X("shorten-labels", "Looks for excessively long labels and shortens them.", false, false);
