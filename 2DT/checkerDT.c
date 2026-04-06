/*--------------------------------------------------------------------*/
/* checkerDT.c                                                        */
/* Author:                                                            */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "checkerDT.h"
#include "dynarray.h"
#include "path.h"



/* see checkerDT.h for specification */
boolean CheckerDT_Node_isValid(Node_T oNNode) {
   Node_T oNParent;
   Path_T oPNPath;
   Path_T oPPPath;

   /* Sample check: a NULL pointer is not a valid node */
   if(oNNode == NULL) {
      fprintf(stderr, "A node is a NULL pointer\n");
      return FALSE;
   }

   /* Sample check: parent's path must be the longest possible
      proper prefix of the node's path */
   oNParent = Node_getParent(oNNode);
   if(oNParent != NULL) {
      oPNPath = Node_getPath(oNNode);
      oPPPath = Node_getPath(oNParent);

      if(Path_getSharedPrefixDepth(oPNPath, oPPPath) !=
         Path_getDepth(oPNPath) - 1) {
         fprintf(stderr, "P-C nodes don't have P-C paths: (%s) (%s)\n",
                 Path_getPathname(oPPPath), Path_getPathname(oPNPath));
         return FALSE;
      }

   }

   return TRUE;
}

/*
   Performs a pre-order traversal of the tree rooted at oNNode.
   Returns FALSE if a broken invariant is found and
   returns TRUE otherwise.

   You may want to change this function's return type or
   parameter list to facilitate constructing your checks.
   If you do, you should update this function comment.
*/
static boolean CheckerDT_treeCheck(Node_T oNNode, size_t *totalCount) {
   size_t ulIndex;
   Node_T prevChild = NULL;

   assert(totalCount != NULL);

   if(oNNode == NULL) {
      *totalCount = 0;
      return TRUE;
   }

   *totalCount = 1;

   if(!CheckerDT_Node_isValid(oNNode))
      return FALSE;
   
   if(Node_getParent(oNNode) == NULL) {
      if(Path_getDepth(Node_getPath(oNNode)) != 1) {
         fprintf(stderr, "depth of root node is not 1\n");
         return FALSE;
      }
   }
      
   if(Path_getDepth(Node_getPath(oNNode)) == 1) {
      if(Node_getParent(oNNode) != NULL) {
         fprintf(stderr, "parent node of root node is not NULL\n");
         return FALSE;
      }
   }

   for(ulIndex = 0; ulIndex < Node_getNumChildren(oNNode); ulIndex++) {
      Node_T oNChild = NULL;
      int iStatus = Node_getChild(oNNode, ulIndex, &oNChild);
      size_t i;
      size_t childCount = 0;

      if(iStatus != SUCCESS) {
         fprintf(stderr, "getNumChildren claims more children than getChild returns\n");
         return FALSE;
      }

      if (prevChild != NULL) {
         if(Path_comparePath(Node_getPath(prevChild),
                       Node_getPath(oNChild)) >= 0) {
            fprintf(stderr, "children are not in lexicographic order\n");
            return FALSE;
         }
      }

      if(Node_getParent(oNChild) != oNNode) {
         fprintf(stderr, "children of the same parent node does not share the same parent\n");
         return FALSE;
      }

      if(Path_getDepth(Node_getPath(oNNode)) !=
         Path_getDepth(Node_getPath(oNChild)) - 1) {
         fprintf(stderr, "child node depth is not correct, should be depth of parent + 1\n");
         return FALSE;
      }

      for(i = 0; i < Node_getNumChildren(oNNode); i++) {
         Node_T child = NULL;
         int iStatus2 = Node_getChild(oNNode, i, &child);

         if(iStatus2 != SUCCESS) {
            fprintf(stderr, "getNumChildren claims more children than getChild returns\n");
            return FALSE;
         }

         if(i != ulIndex) {
            if(Path_comparePath(Node_getPath(child),
                                Node_getPath(oNChild)) == 0) {
               fprintf(stderr, "different children of the same parent share the same path\n");
               return FALSE;
            }
         }
      }

      prevChild = oNChild;

      if(!CheckerDT_treeCheck(oNChild, &childCount))
         return FALSE;

      *totalCount += childCount;
   }

   return TRUE;
}

/* see checkerDT.h for specification */
boolean CheckerDT_isValid(boolean bIsInitialized, Node_T oNRoot,
                          size_t ulCount) {
   size_t count = 0;

   if(!bIsInitialized) {
      if(ulCount != 0) {
         fprintf(stderr, "Not initialized, but count is not 0\n");
         return FALSE;
      }

      if(oNRoot != NULL) {
         fprintf(stderr, "Not initialized, but root is not NULL\n");
         return FALSE;
      }
   }

   if(!CheckerDT_treeCheck(oNRoot, &count))
      return FALSE;
   
   if(count != ulCount) {
      fprintf(stderr, "stored node count does not match actual tree size\n");
      return FALSE;
   }

   return TRUE;
}
