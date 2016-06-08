#include "llvm/Pass.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/CFG.h"

#include <sstream>

#include <set>

using namespace llvm;

namespace
{

  
  struct Info
  {
    Function *function;
    std::map<Function*,std::vector<CallInst*> > directCalls;
    std::set<Function*> transitiveCalls;
    bool isRecursive;
  };

  struct Derecurse : public ModulePass {

    static char ID;

    std::map<Function*,Info> m_info;

    std::map<Value*,std::string> m_tmpName;

    std::string getName(Value *x)
    {
      static unsigned unq=0;
      
      if(m_tmpName.find(x)==m_tmpName.end()){
	std::stringstream tmp;
	
	std::string name=x->getName();
	if(!name.empty()){
	  if(isa<Instruction>(x)){
	    tmp<<"%"<<name;
	  }else if(isa<Argument>(x)){
	    tmp<<"%"<<name;
	  }else if(isa<ConstantInt>(x)){
	    tmp<<dyn_cast<ConstantInt>(x)->getSExtValue();
	  }else{
	    tmp<<"@"<<name;
	  }
	}else{
	  unsigned id=unq++;
	  tmp<<"%tt"<<id;
	}
	name=tmp.str();
	m_tmpName[x]=name;
      }
      return m_tmpName[x];
    }

    Derecurse() : ModulePass(ID) {}


    void buildDirectCallGraph(Module &m)
    {
      /* This is not using CallGraph as it always crashed or hung in some
	 wierd way while trying to enumerate call-sites.

	 I have no idea what I'm doing, probably this pass should rely on
	 some kind of CFG pre-pass.
      */
      
      for(auto &f : m){
	Info info;
	info.function=&f;

	for(auto &bb : f){
	  for(auto &i : bb){
	    if(dyn_cast<InvokeInst>(&i)){
	      throw std::runtime_error("Derecurse : invoke instruction not currently supported.");
	    }
	    
	    auto ci = dyn_cast<CallInst>(&i);
	    if(ci){
	      auto target=ci->getCalledFunction();
	      assert(target);
	      info.directCalls[target].push_back(ci);
	    }
	  }
	}

	m_info[&f]=info;
      }

      for(const auto &i : m_info){
	errs()<<i.first->getName()<<"\n";
	for(const auto &cc : i.second.directCalls){
	  errs()<<"  calls "<<cc.first->getName()<<"\n";
	  for(const auto &ci : cc.second){
	    errs()<<"    at inst "<<ci<<"\n";
	  }
	}
      }
    }

    void connectTransitiveCalls()
    {
      for(auto &info : m_info){
	for(auto &call : info.second.directCalls){
	  info.second.transitiveCalls.insert(call.first);
	}
      }
      
      while(1){
	bool changed=false;

	for(auto &caller : m_info){
	  for(auto &calls : caller.second.transitiveCalls){
	    for(auto &indirect : m_info[calls].transitiveCalls){
	      changed = changed || caller.second.transitiveCalls.insert(indirect).second;
	    }
	  }
	}

	if(!changed)
	  break;
      }
    }

    void findRecursion()
    {
      for(auto &info : m_info){
	if(info.second.transitiveCalls.find(info.first)!=info.second.transitiveCalls.end()){
	  errs()<<info.first->getName()<<" Recursive\n";
	  info.second.isRecursive=true;
	}else{
	  info.second.isRecursive=false;
	}
      }
    }

      /* 
    A function is recursive if there exists any chain of function calls which
    results in a call to itself.

    So if we have f->g->f then both f and g are recursive.
    If we also have f->g->z, then z is not recursive.

    A nest is a set of functions which contain mutually recursive calls.
     - All functions in the nest are recursive 
     - For each call made by a function in the nest:
       - If the callee can call any function in the nest then the callee must also be in the nest

     Each function can be in at most one nest, as if it appeared in two
     nests then by definition those nests would be joined.
    
     There may exist multiple nests, for example:
      (f->g->f) -> (x->y->x)
     however, if there were a call from x or y back into f or g then
     it would have to be part of the same nest.

   */
    void buildNests()
    {
      std::map<Function*,std::set<Function*> > nests;
      for(auto &f : m_info){
	if(f.second.isRecursive){
	  nests[f.first].insert(f.first);

	  for(auto &c : m_info[f.first].directCalls){
	    auto &callee=m_info[c.first];
	    if(callee.transitiveCalls.find(f.first)!=callee.transitiveCalls.end()){
	      nests[f.first].insert(c.first);
	    }
	  }
	}
      }

      while(1){
	auto prevNests = nests;

	for(auto &f : nests){
	  for(auto &links : f.second){
	    auto &out=nests[links];
	    f.second.insert(out.begin(), out.end());
	  }
	}

	if(prevNests==nests)
	  break;
      }

      std::set<std::set<Function*> > res;
      for(auto &f : nests){
	res.insert(f.second);
      }

      for(auto &f : res){
	errs()<<"Set : ";
	for(auto &ff : f){
	  errs()<<" "<<ff->getName();
	}
	errs()<<"\n";
      }
    }

    /* Return all basic blocks that are reachable after this instruction
       TODO: Use a proper analysis!
     */
    std::vector<Instruction*> findReachableInstructions(Instruction *root)
    {
      std::set<BasicBlock*> seen;
      std::vector<BasicBlock*> todo;

      std::vector<Instruction*> result;

      auto process_inst=[&](Instruction *curr){
	BasicBlock::iterator it(curr);
	
	while(1){
	  ///	  errs()<<getName(curr)<<"\n";
	  
	  if((Instruction*)it==root)
	    break;
	  
	  result.push_back(it);
	  TerminatorInst *term=dyn_cast<TerminatorInst>(it);
	  if(term){
	    for(unsigned i=0;i<term->getNumSuccessors();i++){
	      auto succ = term->getSuccessor(i);
	      if( seen.find(succ) == seen.end()){
		todo.push_back(succ);
	      }
	    }
	    ++it;
	    assert(it==curr->getParent()->end());
	    break;
	  }else{
	    ++it;
	  }
	}
      };
      
      // Put everything on current basic block in the list
      {
	BasicBlock::iterator it(root);
	++it;
	if(it != root->getParent()->end() ){
	  process_inst(it);
	}
      }

      while(!todo.empty()){
	BasicBlock *bb=todo.back();
	todo.pop_back();
	process_inst(bb->begin()); 
      }

      return result;
    }

    bool isDynamic(Value *u)
    {
      if(isa<Argument>(u)){
	return true;
      }else if(isa<Constant>(u)){
	return false;
      }else if(isa<BinaryOperator>(u)){
	return true;
      }else if(isa<CallInst>(u)){
	return true; // It might be void, but then nothing will use it
      }else if(isa<CmpInst>(u)){
	return true;
      }else if(isa<ExtractElementInst>(u)){
	return true;
      }else if(isa<GetElementPtrInst>(u)){
	return true;
      }else if(isa<PHINode>(u)){
	return true;
      }else if(isa<SelectInst>(u)){
	return true;
      }else if(isa<UnaryInstruction>(u)){
	return true;
      }else{
	return false;
      }
    }


        /* Find all the variables which are live _after_ this
       instruction executes. The intent is to call this on
       a CallInst in order to work out what needs to be preserved
       across the call.
    */
    std::set<Value *> findLiveValues(Instruction *root)
    {
      std::set<Value*> liveOperands;
      std::set<Value*> newOperands;
      auto reachable=findReachableInstructions(root);
      newOperands.insert(root); // The root instruction is not needed before...
      
      for(auto &inst : reachable){
	//errs()<<"        "<<inst<<", "<<getName(inst)<<" : "<<inst->getOpcodeName();
	for(auto &o : inst->operands()){
	  //errs()<<" "<<getName(o.get());
	  if(newOperands.find(o.get())==newOperands.end()){
	    if(isDynamic(o.get())){
	      liveOperands.insert(o.get());
	    }
	  }
	}
	newOperands.insert(inst);
	//errs()<<"\n";
      }
      return liveOperands;
    }

    /*
      When rewriting we have to be careful about phi nodes at the top of basic-blocks.
      Given:

      bb0 : %x = 6
            jmp bb1

      bb1 : %1 = add %x, 1
            %2 = call f, %1
            jmp bb1

      We might transform to:

      bb1 : %1 = add %x, 1
            store g_x, %x
            %2 = call f, %1
	    %xx = load g_x
            jmp bb1

      There is now a conflict at the top of bb1, so we need to insert a phi node:

      bb1 : %phi_x = phi [%x,bb2],[%xx,bb0]
            %1 = add %x, 1
            store g_x, %x
            %2 = call f, %1
	    %xx = load g_x
            jmp bb1

      Being more complicated:

      bb0:  %x = 5
            jmp bb1

      bb1:  %1 = add %x, 1
            %2 = call f, %1
	    bra %2, bb1, bb2

      bb2:  ...
            jmp bb1


      Rewrite to persist:

      bb1:  %1 = add %x, 1
            store g_x, %x
            %2 = call f, %1
	    %xx = load g_x
	    bra %2, bb1, bb2

      Propogate new value via bb1

      bb1:  %phi_x = phi [%x,bb0],[%xx,bb1],[%x,bb2]
            %1 = add %phi_x, 1
            store g_x, %phi_x
            %2 = call f, %1
	    %xx = load g_x
	    bra %2, bb1, bb2

      Propagate new value via bb2:

      bb1:  
            %phi_x = phi [%x,bb0],[%xx,bb1],[%xx,bb2]
            %1 = add %phi_x, 1
            store g_x, %x
            %2 = call f, %1
	    %xx = load g_x
	    bra %2, bb1, bb2

======================================================================================

      STATUS: Currently this is broken, as the analysis for phis doesn't
      work.

      make test_rec5_analysis


=======================================================================================
     */

    std::string makePhiName()
    {
      static unsigned unq=0;
      std::stringstream tmp;
      tmp<<"phi_"<<unq;
      ++unq;
      return tmp.str();
    }
    
    void rewriteValueAfterPoint(BasicBlock *bb, BasicBlock *pred, Value *oldVal, Value *newVal, std::map<BasicBlock*,PHINode *> &seen)
    {
      errs()<<"      rewrite : "<<getName(bb)<<" <- "<<getName(pred)<<" ; "<<getName(oldVal)<<" -> "<<getName(newVal)<<"\n";
      
      if(seen.find(bb)!=seen.end()){
	errs()<<"      seen\n";
	PHINode *phi=seen[bb];
	if(phi){
	  Value *v=phi->getIncomingValueForBlock(pred);
	  if(v==oldVal){
	    phi->removeIncomingValue(pred, false);
	    phi->addIncoming(newVal, pred);
	    errs()<<"     update phi : "<<getName(phi)<<"\n";
	  }
	  for(unsigned i=0;i<phi->getNumIncomingValues();i++){
	    errs()<<"            ["<<getName(phi->getIncomingValue(i))<<","<<getName(phi->getIncomingBlock(i))<<"]\n";
	  }
	}
	return;
      }
      	
	// We are arriving from some other block
	// If the arrival is conditional, there may be some other inward arcs, which represent other
	// values. If we have something like:
	//
	// int x=5;
	// while(...){
        //    x=x+1;
	//    f();  // recursive
	// }
	//
	// Then a the top of the loop we have the original value of x to worry about.

      int nPreds=std::distance(pred_begin(bb), pred_end(bb));

      PHINode *phi=0;
      

      {
	// We'll insert a phi node (it may get deleted if not needed)
	phi=PHINode::Create(newVal->getType(), nPreds,makePhiName(), bb->begin());
	auto it=pred_begin(bb), end=pred_end(bb);
	while(it!=end){
	  if(pred==*it){
	    phi->addIncoming(newVal, *it);
	  }else{
	    phi->addIncoming(oldVal, *it);
	  }
	  ++it;
	}
	
	newVal=phi;

	errs()<<"          new phi : "<<getName(phi)<<"\n";
	for(unsigned i=0;i<phi->getNumIncomingValues();i++){
	  errs()<<"            ["<<getName(phi->getIncomingValue(i))<<","<<getName(phi->getIncomingBlock(i))<<"]\n";
	}
      }
      seen[bb]=phi;
    
    
      auto it=bb->begin();
      if(phi!=0){
	assert((Instruction*)it==phi);
	++it;
      }
      
      while(it!=bb->end()){
	PHINode *pit=dyn_cast<PHINode>(it);
	if(pit){
	  for(unsigned i=0;i<pit->getNumIncomingValues();i++){
	    if(pit->getIncomingValue(i)==oldVal && pit->getIncomingBlock(i)==pred){
	      pit->setIncomingValue(i, newVal);
	    }
	  }
	}else{
	  for(unsigned i=0;i<it->getNumOperands();i++){
	    if(it->getOperand(i)==oldVal){
	      it->setOperand(i,newVal);
	    }
	  }
	}
	
	if((Instruction*)it==oldVal)
	  break;
	
	if(isa<TerminatorInst>(it)){
	  auto ti=dyn_cast<TerminatorInst>(it);
	  for(unsigned i=0;i<ti->getNumSuccessors();i++){
	    rewriteValueAfterPoint(ti->getSuccessor(i), bb, oldVal, newVal, seen);
	  }
	}
	
	++it;
      }
    }
    
    void rewriteValueAfterPoint(Instruction *instr, Value *oldVal, Value *newVal)
    {
      errs()<<"      rewrite : "<<getName(instr->getParent())<<" ; "<<getName(oldVal)<<" -> "<<getName(newVal)<<"\n";
      
      BasicBlock::iterator it(instr);
      ++it;

      BasicBlock *start=instr->getParent();

      std::map<BasicBlock*,PHINode*> seen;
      while(it!=start->end()){
	for(unsigned i=0;i<it->getNumOperands();i++){
	  if(it->getOperand(i)==oldVal){
	    it->setOperand(i,newVal);
	  }
	}

	if((Instruction*)it==oldVal)
	  break;

	if(isa<TerminatorInst>(it)){
	  auto ti=dyn_cast<TerminatorInst>(it);
	  for(unsigned i=0;i<ti->getNumSuccessors();i++){
	    rewriteValueAfterPoint(ti->getSuccessor(i), start, oldVal, newVal, seen);
	  }
	}

	++it;
      }

      while(1){
	bool changed=false;
	for(auto &s : seen){
	  if(s.second){
	    if(s.second->getNumUses()==0){
	      s.second->eraseFromParent();
	      s.second=0;
	      changed=true;
	    }
	  }
	}
	if(!changed)
	  break;
      }
    }
      
    
    void dumpRecursiveCalls()
    {
      for(auto &fi : m_info){
	if(!fi.second.isRecursive)
	  continue;

	errs()<<"  Recursive calls by "<<fi.first->getName()<<"\n";
	for(auto &outgoing : fi.second.directCalls){
	  errs()<<"    calls to "<<outgoing.first->getName()<<"\n";
	  for(auto &instr : outgoing.second){
	    errs()<<"      instruction "<<instr<<" : "<<getName(instr)<<"\n";

	    auto liveOperands = findLiveValues(instr);

	    errs()<<"      used = ";
	    for(auto &ops : liveOperands){
	      errs()<<" "<<getName(ops);
	    }
	    errs()<<"\n";
	  }
	}
      }
    }

    std::string makeBackingName(Value *a)
    {
      static unsigned unq=0;
      
      std::stringstream tmp;
      if(isa<Argument>(a)){
	tmp<<"rec_"<<std::string(dyn_cast<Argument>(a)->getParent()->getName());
      }else{
	tmp<<"rec_"<<std::string(dyn_cast<Instruction>(a)->getParent()->getParent()->getName());
      }
      std::string xx=getName(a);
      if(xx[0]!='%')
	throw std::runtime_error("Attempting to save non % variable?");
      tmp<<"_"<<xx.substr(1)<<"_u"<<unq;
      ++unq;
      return tmp.str();
    }
    
    void rewriteRecursiveInstruction(Instruction *instr)
    {
      auto live=findLiveValues(instr);

      auto &context=instr->getParent()->getParent()->getContext();
      auto zero=ConstantInt::get(Type::getInt32Ty(context), 0);

      for(auto &val : live){
	std::string name=makeBackingName(val);

	errs()<<"        adding backing : "<<name<<"\n";
	auto g=new GlobalVariable(*instr->getParent()->getParent()->getParent(), instr->getType(), false, GlobalValue::ExternalLinkage, nullptr, name);

	IRBuilder<> builder(instr);

	auto store=builder.CreateStore(val, g);

	Instruction *follow=++BasicBlock::iterator(instr);

	IRBuilder<> builder2(follow);

	auto load=builder2.CreateLoad(g);

	rewriteValueAfterPoint(load, val, load);
      }
    }

    void rewriteRecursiveInstructions()
    {
      for(auto &fi : m_info){
	if(!fi.second.isRecursive)
	  continue;

	errs()<<"  Recursive calls by "<<fi.first->getName()<<"\n";
	for(auto &outgoing : fi.second.directCalls){
	  errs()<<"    calls to "<<outgoing.first->getName()<<"\n";
	  for(auto &instr : outgoing.second){
	    errs()<<"      instruction "<<instr<<" : "<<getName(instr)<<"\n";
	    rewriteRecursiveInstruction(instr);
	  }
	}
      }
    }
    

    bool runOnModule(Module &m) override {
      buildDirectCallGraph(m);
      connectTransitiveCalls();
      findRecursion();
      buildNests();

      dumpRecursiveCalls();

      rewriteRecursiveInstructions();

      m.print(errs(),nullptr);
      
      return true;
    }

    bool runOnFunction(Function &f) {
      errs() << "Hello: ";
      errs().write_escaped(f.getName()) << "\n";

      
      
      return false;
    };
  };

};

char Derecurse::ID = 0;

static RegisterPass<Derecurse> X("derecurse", "De-Recurse Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);

