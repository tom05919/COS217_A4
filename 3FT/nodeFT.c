/*--------------------------------------------------------------------*/
/* nodeFT.c                                                           */
/* Author: Ty Lipscomb and Tom Wang                                   */
/*--------------------------------------------------------------------*/

#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "dynarray.h"
#include "a4def.h"
#include "path.h"
#include "nodeFT.h"

struct node {
   /* the object corresponding to the node's absolute path */
   Path_T oPPath;
   /* this node's parent */
   Node_T oNParent;
   /* the object containing links to this node's children */
   DynArray_T oDChildren;

    /* Used to differentiate between directories and files in the same node structure. */
   NodeType uType;

	/* the contents of the node FILE type only, otherwise NULL */
   void *pvContents;
	/* the length of the file contents. Should be 0 for directories. */
   size_t ulLength;

};

/* come back to this and add comments */
boolean Node_find(Node_T oNParent, Path_T oPPath, NodeType uType, size_t *pulIndex) {
   struct node sDummy;
   sDummy.oPPath = oPPath;
   sDummy.uType = uType;
   
   return DynArray_bsearch(oNParent->oDChildren, &sDummy, pulIndex, (int(*)(const void*, const void*))Node_compare);
}

/*
  Built upon the original node logic.
  Can take in either a directory or file node, determined by NodeType.
  If uType is TYPE_DIR, then pvContents and ulLength should be set to NULL (unused).
  If uType is TYPE_FILE, then pvContents and ulLength should be set to the file's contents and length, respectively.
  Both will take in oPPath and oNParent as before.
  Returns SUCCESS if the new node is created successfully.
*/
int Node_new(Path_T oPPath, Node_T oNParent, NodeType uType, Node_T *poNResult, size_t ulLength, void *pvContents) {
   struct node *psNew;
   Path_T oPParentPath = NULL;
   Path_T oPNewPath = NULL;
   size_t ulParentDepth;
   size_t ulIndex;
   int iStatus;
   
   assert(oPPath != NULL);
   assert(uType == TYPE_DIR || uType == TYPE_FILE);

	/* logic for validating the contents and length based on node type */
   assert((uType == TYPE_DIR && pvContents == NULL && ulLength == 0) ||
          (uType == TYPE_FILE && pvContents == NULL && ulLength == 0) ||
			 (uType == TYPE_FILE && pvContents != NULL && ulLength > 0));

	if (oNParent != NULL && oNParent->uType == TYPE_FILE) {
		/* files cannot be the root */
		return NOT_A_DIRECTORY;
	}

    /* allocate space for a new node */
   psNew = malloc(sizeof(struct node));
   if(psNew == NULL) {
      *poNResult = NULL;
      return MEMORY_ERROR;
   }

	/* set the new node's path */
   iStatus = Path_dup(oPPath, &oPNewPath);
   if(iStatus != SUCCESS) {
      free(psNew);
      *poNResult = NULL;
      return iStatus;
   }
   psNew->oPPath = oPNewPath;

	/* validate and set the new node's parent */
   if(oNParent != NULL) {
      size_t ulSharedDepth;

      oPParentPath = oNParent->oPPath;
      ulParentDepth = Path_getDepth(oPParentPath);
      ulSharedDepth = Path_getSharedPrefixDepth(psNew->oPPath,
                                                oPParentPath);

      /* parent must be an ancestor of child */
      if(ulSharedDepth < ulParentDepth) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poNResult = NULL;
         return CONFLICTING_PATH;
      }

      /* parent must be exactly one level up from child */
      if(Path_getDepth(psNew->oPPath) != ulParentDepth + 1) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poNResult = NULL;
         return NO_SUCH_PATH;
      }

      /* parent must not already have child with this path */
      if(Node_hasChild(oNParent, oPPath, &ulIndex)) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poNResult = NULL;
         return ALREADY_IN_TREE;
      }
      /* updates ulIndex to the spot it should be in case hasChild messes it up */
      Node_find(oNParent, oPPath, uType, &ulIndex);
   }
   else {
		/* new node must be root AND dir */
		if (uType == TYPE_FILE) {
			Path_free(psNew->oPPath);
			free(psNew);
			*poNResult = NULL;
			return CONFLICTING_PATH;
		}

      /* can only create one "level" at a time */
      if(Path_getDepth(psNew->oPPath) != 1) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poNResult = NULL;
         return NO_SUCH_PATH;
      }
   }
	psNew->oNParent = oNParent;

	/* initialize the new node based on its type */
	if (uType == TYPE_DIR) {
		psNew->oDChildren = DynArray_new(0);
		if(psNew->oDChildren == NULL) {
			Path_free(psNew->oPPath);
			free(psNew);
			*poNResult = NULL;
			return MEMORY_ERROR;
		}
		psNew->pvContents = NULL;
		psNew->ulLength = 0;
		psNew->uType = uType;
	}
	else {
		/* if file, NO children allowed and the file will have contents and ulLength*/
		psNew->oDChildren = NULL;
		psNew->ulLength = ulLength;
		psNew->uType = uType;

      if (ulLength > 0 && pvContents != NULL) {
         psNew->pvContents = malloc(ulLength);
         
         if (psNew->pvContents == NULL) {
            Path_free(psNew->oPPath);
            free(psNew);
            *poNResult = NULL;
            return MEMORY_ERROR;
         }
         /* make an ACTUAL copy of the memory to ours, otherwise contents could be freed by client later*/
         memcpy(psNew->pvContents, pvContents, ulLength);
      }
      else {
         /* empty file*/
         psNew->pvContents = NULL;
      }
	}

	/* Link into parent's children list */
   if(oNParent != NULL) {
      iStatus = Node_addChild(oNParent, psNew, ulIndex);

      if(iStatus != SUCCESS) {
         if (uType == TYPE_DIR) {
            DynArray_free(psNew->oDChildren);
         }
         else {
            free(psNew->pvContents);
         }
         Path_free(psNew->oPPath);
         free(psNew);
         *poNResult = NULL;
         return iStatus;
      }

   }

	*poNResult = psNew;

   return SUCCESS;
}


/* new functions defined before */
/* Returns the NodeType of a provided node (oNNode)*/
NodeType Node_getType(Node_T oNNode) {
   assert(oNNode != NULL);

   return oNNode->uType;
}

/* Returns a pointer to the contents of a provided file node (oNNode) MUST BE FILE TYPE OTHERWISE RETURN ERROR */
void *Node_getContents(Node_T oNNode) {
   assert(oNNode != NULL);
   assert(oNNode->uType == TYPE_FILE);
   return oNNode->pvContents;
}

/* returns file length. Cannot call on directory nodes otherwise logic error*/
size_t Node_getContentsLength(Node_T oNNode) { 
   assert(oNNode != NULL);
   assert(oNNode->uType == TYPE_FILE);

   return oNNode->ulLength;
}

/* updates the file contents. Returns the old contents on success, ERROR on failure, ONLY FOR FILES*/
void *Node_setContents(Node_T oNNode, void *pvNewContents, size_t ulNewLength) {
   void *pvOldContents;
   assert(oNNode != NULL);
   assert(oNNode->uType == TYPE_FILE);
   pvOldContents = oNNode->pvContents;

  if (ulNewLength > 0 && pvNewContents != NULL) {
      oNNode->pvContents = malloc(ulNewLength);

      if (oNNode->pvContents == NULL) {
         oNNode->pvContents = pvOldContents;
         return NULL;
      } 

      memcpy(oNNode->pvContents, pvNewContents, ulNewLength);
      oNNode->ulLength = ulNewLength;

  } else {
      /* if new contents is empty, free old contents and set to NULL */
      oNNode->pvContents = NULL;
      oNNode->ulLength = 0;
   }

   return pvOldContents;
}

/*
  Destroys and frees all memory allocated for the subtree rooted at
  oNNode, i.e., deletes this node and all its descendents. Returns the
  number of nodes deleted.
*/
size_t Node_free(Node_T oNNode) {
   size_t ulCount;
   size_t ulNumChildren;
   size_t i;
   Node_T oChild;
   assert(oNNode != NULL);

   ulCount = 1;

   if (oNNode->uType == TYPE_DIR) {
      ulNumChildren = DynArray_getLength(oNNode->oDChildren);

      for(i = 0; i < ulNumChildren; i++) {
         oChild = DynArray_get(oNNode->oDChildren, i);

         /* recursive behavior through all the children*/
         ulCount += Node_free(oChild);
      }
      /* free the remaining child*/
      DynArray_free(oNNode->oDChildren);
   } else {
      if(oNNode->pvContents != NULL) {
         free(oNNode->pvContents);
      }
   }

   Path_free(oNNode->oPPath);
   free(oNNode);

   return ulCount;
   
}

/* Returns the path object representing oNNode's absolute path. */
Path_T Node_getPath(Node_T oNNode) {
   assert(oNNode != NULL);

   return oNNode->oPPath;
}

/*
  Returns TRUE if oNParent has a child with path oPPath. Returns
  FALSE if it does not.

  If oNParent has such a child, stores in *pulChildID the child's
  identifier (as used in Node_getChild). If oNParent does not have
  such a child, stores in *pulChildID the identifier that such a
  child _would_ have if inserted.
*/
boolean Node_hasChild(Node_T oNParent, Path_T oPPath, size_t *pulChildID) {
   struct node TempNode;

   assert(oNParent != NULL);
   assert(oPPath != NULL);       
   assert(oNParent->uType == TYPE_DIR);
   assert(pulChildID != NULL);

   TempNode.oPPath = oPPath;

   /* try it assuming file type*/
   TempNode.uType = TYPE_FILE;
   if (DynArray_bsearch(oNParent->oDChildren, &TempNode, pulChildID, (int(*)(const void*, const void*))Node_compare)) {
      return TRUE;
   } 

   /* for DIR type*/
   TempNode.uType = TYPE_DIR;
   return DynArray_bsearch(oNParent->oDChildren, &TempNode, pulChildID, (int(*)(const void*, const void*))Node_compare);

}

/* come back add comments */
int Node_addChild(Node_T oNParent, Node_T oNChild, size_t ulIndex) {
   assert(oNParent != NULL);
   assert(oNChild != NULL);
   assert(oNParent->uType == TYPE_DIR);

   if (DynArray_addAt(oNParent->oDChildren, ulIndex, oNChild) == 0)
      return MEMORY_ERROR;

   return SUCCESS;
}

/*
  Returns an int SUCCESS status and sets *poNResult to be the child
  node of oNParent with identifier ulChildID, if one exists.
  Otherwise, sets *poNResult to NULL and returns status:
  * NO_SUCH_PATH if ulChildID is not a valid child for oNParent
*/
int Node_getChild(Node_T oNParent, size_t ulChildID, Node_T *poNResult) {
   assert(oNParent != NULL);
   assert(poNResult != NULL);

   /* only directories have children*/
   if (oNParent->uType != TYPE_DIR) {
      *poNResult = NULL;
      return NOT_A_DIRECTORY;
   }

   if (ulChildID >= DynArray_getLength(oNParent->oDChildren)) {
      *poNResult = NULL;
      return NO_SUCH_PATH;
   }

   /* outputted child */
   *poNResult = DynArray_get(oNParent->oDChildren, ulChildID);

   return SUCCESS;
}

/* Returns the number of children that oNParent has. */
size_t Node_getNumChildren(Node_T oNParent) {
   assert(oNParent != NULL);
   assert(oNParent->uType == TYPE_DIR);

   return DynArray_getLength(oNParent->oDChildren);
}

/* Remove (unlink) the child at ulChildID from oNParent's children array.
   Caller is responsible for freeing the node. */
void Node_unlinkChild(Node_T oNParent, size_t ulChildID) {
   assert(oNParent != NULL);
   assert(oNParent->uType == TYPE_DIR);
   assert(ulChildID < DynArray_getLength(oNParent->oDChildren));

   DynArray_removeAt(oNParent->oDChildren, ulChildID);
}

/*
  Returns a the parent node of oNNode.
  Returns NULL if oNNode is the root and thus has no parent.
*/
Node_T Node_getParent(Node_T oNNode) {
   assert(oNNode != NULL);

   return oNNode->oNParent;
}

/*
  Compares oNFirst and oNSecond lexicographically based on their paths.
  Returns <0, 0, or >0 if onFirst is "less than", "equal to", or
  "greater than" oNSecond, respectively.
*/
int Node_compare(Node_T oNFirst, Node_T oNSecond) {
   assert(oNFirst != NULL);
   assert(oNSecond != NULL);

   /* since files come before directories, return -1*/
   if (oNFirst->uType == TYPE_FILE && oNSecond->uType == TYPE_DIR) {
      return -1; /* first is less than second*/
   }

   /* since files come before directories, return 1*/
   if (oNFirst->uType == TYPE_DIR && oNSecond->uType == TYPE_FILE) {
      return 1; /* first is greater than second */
   }

   /* if not opposing types, return compare path like normal */
   return Path_comparePath(Node_getPath(oNFirst), Node_getPath(oNSecond));
}

/*
  Returns a string representation for oNNode, or NULL if
  there is an allocation error.

  Allocates memory for the returned string, which is then owned by
  the caller!
*/
char *Node_toString(Node_T oNNode) {
   char *pcResult;
   const char *pcPathString;
   size_t ulLength;

   assert(oNNode != NULL);

   pcPathString = Path_getPathname(oNNode->oPPath);

   /* account for newline / null*/
   ulLength = strlen(pcPathString) + 2;
   pcResult = malloc(ulLength);

   /* copy path string to the result*/
   strcpy(pcResult, pcPathString);

   if (oNNode->uType == TYPE_DIR) {
      size_t i;
      size_t ulNumChildren = DynArray_getLength(oNNode->oDChildren);


      /* newline for directory name before children*/
      strcat(pcResult, "\n");

      for (i = 0; i < ulNumChildren; i++) {
         Node_T oChild = DynArray_get(oNNode->oDChildren, i);
         char *pcChildString = Node_toString(oChild);
         char *pcTemp;

         if (pcChildString != NULL) { 
            /* update length with current result + child string + newline (this is recursive)*/
            ulLength += strlen(pcChildString) +1;

            /* expanding logic with memeory error check*/
            pcTemp = realloc(pcResult, ulLength);
            if (pcTemp == NULL) {
               free(pcChildString);
               free(pcResult);
               return NULL;
            }
            pcResult = pcTemp;

            /* add the child and cleanup child char*/
            strcat(pcResult, pcChildString);
            strcat(pcResult, "\n");
            free(pcChildString);
         }

      }
      /* remove trailing newline*/
      pcResult[strlen(pcResult) - 1] = '\0';
   }

   return pcResult;
}
