// @(#)root/reflex:$Name:  $:$Id: CINTFunctionBuilder.cxx,v 1.2 2005/11/03 15:29:47 roiser Exp $
// Author: Pere Mato 2005

// Copyright CERN, CH-1211 Geneva 23, 2004-2005, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#include "Reflex/Reflex.h"
#include "Reflex/Tools.h"
#include "CINTdefs.h"
#include "CINTFunctionBuilder.h"
#include "CINTScopeBuilder.h"
#include "CINTFunctional.h"
#include "Api.h"

using namespace ROOT::Reflex;
using namespace std;

namespace ROOT { namespace Cintex {
  
  CINTFunctionBuilder::CINTFunctionBuilder(const ROOT::Reflex::Member& m)
  : fFunction(m) { }

  CINTFunctionBuilder::~CINTFunctionBuilder() {
  }

  void CINTFunctionBuilder::Setup() {

    Scope ScopeNth = fFunction.DeclaringScope();
    bool global = ScopeNth.IsTopScope();

    if ( global ) {
      G__lastifuncposition();
    }
    else {
      CINTScopeBuilder::Setup(ScopeNth);
      string sname = ScopeNth.Name(SCOPED);
      int ns_tag = G__search_tagname(sname.c_str(),'n');
      G__tag_memfunc_setup(ns_tag);
    }

    Setup(fFunction);

    if ( global ) {
      G__resetifuncposition();
    }
    else {
      G__tag_memfunc_reset();
    }
 
    return;
  }

  void CINTFunctionBuilder::Setup(const Member& function) {
    Type cl = Type::ByName(function.DeclaringScope().Name(SCOPED));
    int access        = G__PUBLIC;
    int const_ness    = 0;
    int virtuality    = 0;
    int reference     = 0;
    int memory_type   = 1; // G__LOCAL;  // G__AUTO=-1
    int tagnum        = CintTag(function.DeclaringScope().Name(SCOPED));

    //---Alocate a context
    StubContext* stub_context = new StubContext(function, cl);

    //---Function Name and hash value
    string funcname = function.Name();
    int hash, tmp;

    //---Return TypeNth----------------
    Type rt = function.TypeOf().ReturnType();
    reference = rt.IsReference() ? 1 : 0;
    while ( rt.IsTypedef() ) rt = rt.ToType();
    //CINTScopeBuilder::Setup( rt );
    CintTypeDesc ret_desc = CintType( rt );
    char ret_type = rt.IsPointer() ? (ret_desc.first - ('a'-'A')) : ret_desc.first;
    int  ret_tag  = CintTag( ret_desc.second );

    if( function.IsOperator() ) {  
      // remove space between "operator" keywork and the actual operator
      if ( funcname[8] == ' ' && ! isalpha( funcname[9] ) ) 
        funcname = "operator" + funcname.substr(9);
    }
    G__InterfaceMethod stub;
    if( function.IsConstructor() ) {
      //stub = Constructor_stub;
      stub = Allocate_stub_function(stub_context, & Constructor_stub_with_context);
      funcname = G__ClassInfo(tagnum).Name();
      ret_tag = tagnum;
    }
    else if ( function.IsDestructor() ) {
      //stub = Destructor_stub;
      stub = Allocate_stub_function(stub_context, & Destructor_stub_with_context);
      funcname =  "~";
      funcname += G__ClassInfo(tagnum).Name();
    }
    else {
      //stub = Method_stub;
      stub = Allocate_stub_function(stub_context, & Method_stub_with_context);
    }
    if ( function.IsPrivate() )  
      access = G__PRIVATE;
    else if ( function.IsProtected() )
      access = G__PROTECTED;
    else if ( function.IsPublic() )
      access = G__PUBLIC;

    //if ( function.IsConst() )  
    //  const_ness = G__CONSTFUNC;
    if ( function.IsVirtual() )  
      virtuality = 1;
    if ( function.IsStatic() )  
      memory_type += G__CLASSSCOPE;

    string signature = CintSignature(function);
    int nparam = function.TypeOf().FunctionParameterSize();
    //---Cint function hash
    G__hash(funcname, hash, tmp);

    G__usermemfunc_setup( funcname.c_str(),                // function Name
                          hash,                            // function Name hash value
                          stub,                            // method stub function
                          ret_type,                        // return TypeNth (void)
                          ret_tag,                         // return TypeNth tag number
                          -1,                              // TypeNth number
                          reference,                       // reftype
                          nparam,                          // number of paramerters
                          memory_type,                     // memory TypeNth
                          access,                          // access TypeNth
                          const_ness,                      // CV qualifiers
                          signature.c_str(),               // signature
                          (char*)NULL,                     // comment line
                          (void*)NULL,                     // true2pf
                          virtuality,                      // virtuality
                          stub_context                     // user ParameterNth 
                       );

  }
}}
