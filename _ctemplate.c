/*
 * C Template Library 1.0
 *
 * Copyright 2009 Stephen C. Losen.  Distributed under the terms
 * of the GNU General Public License (GPL)
 * 
 * Python port written by Marek Lipert (marek.lipert@gmail.com)
 *   
 */

#include <Python.h>
#include <stdlib.h>
#include <stdio.h>
#include "ctemplate.h"

static char module_docstring[] =
        "This module is a port of C Template Library 1.0 for Python";
static char ctemplate_docstring[] =
        "This function takes two arguments: a template string and a dictionary. It returns compiled template string. Dictionary has to consist of either strings, arrays of such dictionaries (recurrent) or such dictionaries (recurrent). All other types have to be explicitly converted to strings before usage.";

static PyObject *ctemplate_skompiluj_wzorzec(PyObject *self, PyObject *args);
static TMPL_varlist *addDictionaryToVarlist(TMPL_varlist *list,const char* prefix, PyObject *dictionary);

static PyMethodDef module_methods[] = 
{
    {"compile_template", ctemplate_skompiluj_wzorzec, METH_VARARGS, ctemplate_docstring},
        {NULL, NULL, 0, NULL}
};


PyMODINIT_FUNC init_ctemplate(void)
{
    PyObject *m = Py_InitModule3("_ctemplate", module_methods, module_docstring);
        if (m == NULL)
                return;
}

static PyObject *ctemplate_skompiluj_wzorzec(PyObject *self, PyObject *args)
{
  PyObject *main_dictionary = NULL;
  PyObject *retVal = NULL;
  FILE *mainFile,*errorFile;
  char *template = NULL;
  char *result = NULL, *errorString = NULL;
  TMPL_varlist *globals = NULL;
  char *temporaryFileName = malloc(L_tmpnam);
  char *errorFileName = malloc(L_tmpnam);
  int position = 0,freadPosition = -1;
  if (!PyArg_ParseTuple(args, "sO!", &template,&PyDict_Type,&main_dictionary))
                                 return NULL; /* this returns borrowed references */

  globals = addDictionaryToVarlist(NULL,NULL,main_dictionary);
 
  if(!globals && PyErr_Occurred()) return NULL;
  tmpnam(temporaryFileName); /* we need temporary file name for file operations forced by ctemplate */

  if((mainFile = fopen(temporaryFileName,"wr"))==NULL)
  {
   PyErr_SetFromErrno(PyExc_IOError);
   goto onError2;
  }
  tmpnam(errorFileName); /* we need temporary file name for file operations forced by ctemplate */

  if((errorFile = fopen(errorFileName,"wr"))==NULL)
  {
   PyErr_SetFromErrno(PyExc_IOError);
   goto onError;
  }
   
  TMPL_write(NULL,template,NULL,globals,mainFile,errorFile);
  /* we dispose of globals as soon as possible to preserve memory */
  if(globals) TMPL_free_varlist(globals);
  globals = NULL;
  
  fseek(errorFile,0L,SEEK_END);
  
  position = ftell(errorFile);
  
  if(position)
  {
   errorString = malloc(position+1);
   fclose(errorFile);
   if((errorFile = fopen(errorFileName,"r"))==NULL)
   {
     PyErr_SetFromErrno(PyExc_IOError);
     goto onError;
   }

   fread(errorString,1,position,errorFile);
   if(ferror(errorFile))
   {
     PyErr_SetFromErrno(PyExc_IOError);    
   }
   else
   {
    errorString[position]='\0';
    PyErr_SetString(PyExc_SyntaxError,errorString);
   }
   free(errorString);
   errorString = NULL;
   goto onError;
  }
  
  fseek(mainFile,0L,SEEK_END);
  position = ftell(mainFile);
  
  if(position)
  {
   result = malloc(position+1);
   fclose(mainFile);
   if((mainFile = fopen(temporaryFileName,"r"))==NULL)
   {
     PyErr_SetFromErrno(PyExc_IOError);
     goto onError;
   }

   freadPosition = fread(result,1,position,mainFile);
   if(ferror(mainFile))
   {
     PyErr_SetFromErrno(PyExc_IOError);    
     goto onError;
   }
   else
   {
    result[position]='\0';
    retVal = PyString_FromString(result);
    if(!retVal) goto onError;
   }
    if(result) free(result);
    if(errorString) free(errorString);
    fclose(errorFile);
    unlink(errorFileName);
    free(errorFileName);
    fclose(mainFile);
    unlink(temporaryFileName);  
    free(temporaryFileName);
    if(globals) TMPL_free_varlist(globals);
    return retVal;  
  }
  PyErr_SetString(PyExc_IOError,"Empty results file");
  onError:
   if(result) free(result);
   if(errorString) free(errorString);
   fclose(errorFile);
   unlink(errorFileName);
   free(errorFileName);
  onError2:
   if(globals) TMPL_free_varlist(globals);
   fclose(mainFile);
   unlink(temporaryFileName);  
   free(temporaryFileName);
   return NULL;
}



static TMPL_varlist *addDictionaryToVarlist(TMPL_varlist *list,const char* prefix, PyObject *dictionary)
{
  TMPL_varlist *retVal = list;
  TMPL_varlist *loopList = NULL;
  TMPL_loop *aLoop = NULL;  
  PyObject *keys = PyDict_Keys(dictionary); /* new reference */
  Py_ssize_t i = 0, dsize = -1, j = 0, lsize= -1;
  PyObject *key = NULL, *value = NULL;
  PyObject *nestedDict = NULL;
  char *keyChar = NULL, *valChar = NULL;
  char *keyCharTmp = NULL;

  if(!keys) goto onError;
  dsize = PyList_Size(keys);
  for(i=0;i<dsize;i++)
  { /* items numbered from 0 to dsize */
    key = PyList_GetItem(keys,i); /* borrowed reference */
    value = PyDict_GetItem(dictionary,key); /* borrowed reference */
    if(!value || !key) goto onError;
    keyCharTmp = PyString_AsString(key);
    if(!keyCharTmp) goto onError;
    if(prefix && strlen(prefix))
    { /* have prefix */
       keyChar = malloc(strlen(prefix) + /* . */ 1 + strlen(keyCharTmp) + /* \0 */ 1 );
       sprintf(keyChar,"%s.%s",prefix,keyCharTmp);
    }
    else
    { /* no prefix */
       keyChar = malloc(strlen(keyCharTmp)+1);
       strcpy(keyChar,keyCharTmp);
    }    
    if(PyObject_TypeCheck(value,&PyList_Type))
    { /* loop */
     lsize = PyList_Size(value);
     for(j=0;j<lsize;j++)
     { /* items numbered from 0 to lsize */
       nestedDict = PyList_GetItem(value,j); /* borrowed reference */
       if(!nestedDict) goto onError;
       if(!PyObject_TypeCheck(nestedDict,&PyDict_Type))
       { /* array does not contain a dictionary */
        PyErr_SetString(PyExc_AttributeError,"Lists can contain only dictionaries");
        goto onError;
       }
        /* recurrence */
        loopList = addDictionaryToVarlist(NULL,NULL,nestedDict);
        if(!loopList) goto onError;
        aLoop = TMPL_add_varlist(aLoop,loopList);
     }
        if(!aLoop) goto onError;
        retVal = TMPL_add_loop(retVal,keyChar,aLoop);
        aLoop = NULL;
    }
    else
    {
     if(PyObject_TypeCheck(value,&PyDict_Type))
     { /* single subdictionary */
       retVal = addDictionaryToVarlist(retVal,keyChar,value);
     }
     else
     { /* string */
      if(!PyObject_TypeCheck(value,&PyString_Type))
      { /* not a list nor a string - error! */
        PyErr_SetString(PyExc_AttributeError,"Entries in dictionaries can be either strings, lsist or other dictionaries. No other types are permitted");
        goto onError;
      }
      valChar = PyString_AsString(value);
      if(!valChar) goto onError;
      
      retVal = TMPL_add_var(retVal,keyChar,valChar,NULL);
      free(keyChar);
      keyChar = NULL;
     }
    }    
  }
  if(keys) Py_DECREF(keys);
  return retVal;
  onError:
        if(loopList) TMPL_free_varlist(loopList);
        if(keys) Py_DECREF(keys);
        if(keyChar) free(keyChar);
        if(retVal) TMPL_free_varlist(retVal);
        return NULL;
        
}

