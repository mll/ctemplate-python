#!/usr/bin/python
# *
# * C Template Library 1.0
# *
# * Copyright 2009 Stephen C. Losen.  Distributed under the terms
# * of the GNU General Public License (GPL)
# * 
# * Python port written by Marek Lipert (marek.lipert@gmail.com)
# *   
# *




import _ctemplate

d = {"varName":"haha1",
     "aLoop":[{"loopVar":"loopVal 1","loopVar2":"kokoszka"},{"loopVar":"loopVal 2","loopVar2":"kogut"} ],
     "sweet":{"name":"Prince Polo",
              "price":"5.50","isBig":"0"
             }
    }

print _ctemplate.compile_template("""<* This is a comment *>
                                     varName: <TMPL_var name="varName"> 
                                     <TMPL_loop name="aLoop"> 
                                       Loop has var <TMPL_var name="loopVar"> a pieje nam <TMPL_var name="loopVar2">
                                       Loops can be nested!
                                     </TMPL_loop> 
                                     Sweet name: <TMPL_var name="sweet.name">
                                     Sweet price: <TMPL_var name="sweet.price">
                                     <TMPL_if name="sweet.isBig" value="1">
                                      Sweet is big
                                     </TMPL_IF>
                                       """,d)
