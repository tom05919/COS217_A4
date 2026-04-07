/*--------------------------------------------------------------------*/
/* ft.c                                                               */
/* Author: Ty Lipscomb and Tom Wang                                   */
/*--------------------------------------------------------------------*/

#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "path.h"
#include "nodeFT.h"
#include "ft.h"

/* The root node of the file tree. */
static Node_T oNRoot = NULL;

/* Is the data structure initialized? */
static boolean bIsInitialized = FALSE;

/*
  PRIVATE HELPER: FT_traversePath                                    
  Walks down the tree level-by-level to find the node for oPPath.    
  Returns SUCCESS and sets *poNResult if found.                      
  Returns NO_SUCH_PATH or NOT_A_DIRECTORY on failure.                
*/
static int FT_traversePath(Path_T oPPath, Node_T *poNResult) {
   Node_T oNCurr = oNRoot;
   Path_T oPPrefix = NULL;
   size_t i, ulDepth, ulChildID;
   int iStatus;

   assert(oPPath != NULL);
   assert(poNResult != NULL);

   if (oNRoot == NULL) {
      return NO_SUCH_PATH;
   }

   ulDepth = Path_getDepth(oPPath);

   /* Level 1 is the root. We check if the root matches. */
   iStatus = Path_prefix(oPPath, 1, &oPPrefix);
   if (iStatus != SUCCESS) return iStatus;

   if (Path_comparePath(Node_getPath(oNRoot), oPPrefix) != 0) {
      Path_free(oPPrefix);
      return CONFLICTING_PATH;
   }
   Path_free(oPPrefix);

   /* Traverse subsequent levels */
   for (i = 2; i <= ulDepth; i++) {
      iStatus = Path_prefix(oPPath, i, &oPPrefix);
      if (iStatus != SUCCESS) return iStatus;

      /* If we hit a file but need to go deeper, it's an error */
      if (Node_getType(oNCurr) != TYPE_DIR) {
         Path_free(oPPrefix);
         return NOT_A_DIRECTORY;
      }

      /* Check if current directory has this child */
      if (!Node_hasChild(oNCurr, oPPrefix, &ulChildID)) {
         Path_free(oPPrefix);
         return NO_SUCH_PATH;
      }

      /* Move down to the child */
      iStatus = Node_getChild(oNCurr, ulChildID, &oNCurr);
      Path_free(oPPrefix);
      
      if (iStatus != SUCCESS) {
         return iStatus;
      }
   }

   *poNResult = oNCurr;
   return SUCCESS;
}

/*
 PRIVATE HELPER: FT_insertNode                                      
 Centralizes the logic for inserting both Directories and Files.    
 Creates intermediate directories if they do not exist.             
*/
static int FT_insertNode(Path_T oPPath, NodeType uType, void *pvContents, size_t ulLength) {
   Node_T oNCurr = oNRoot;
   size_t i, ulDepth, ulChildID;
   int iStatus;

   ulDepth = Path_getDepth(oPPath);

   /* Special Case: Creating the Root */
   if (oNRoot == NULL) {
      Path_T oPRootPath;
      if (uType == TYPE_FILE) return CONFLICTING_PATH; /* Root can't be a file */
      
      iStatus = Path_prefix(oPPath, 1, &oPRootPath);
      if (iStatus != SUCCESS) return iStatus;

      iStatus = Node_new(oPRootPath, NULL, TYPE_DIR, &oNRoot, 0, NULL);
      Path_free(oPRootPath);
      if (iStatus != SUCCESS) return iStatus;

      oNCurr = oNRoot;
      
      if (ulDepth == 1) return SUCCESS;
   }

   /* Verify root prefix matches */
   {
      Path_T oPRootPath;
      iStatus = Path_prefix(oPPath, 1, &oPRootPath);
      if (iStatus != SUCCESS) return iStatus;
      if (Path_comparePath(Node_getPath(oNRoot), oPRootPath) != 0) {
         Path_free(oPRootPath);
         return CONFLICTING_PATH;
      }
      Path_free(oPRootPath);
   }

   /* Traverse and create missing nodes */
   for (i = 2; i <= ulDepth; i++) {
      Path_T oPPrefix;
      Node_T oNChild;
      NodeType uCurrentType;

      iStatus = Path_prefix(oPPath, i, &oPPrefix);
      if (iStatus != SUCCESS) return iStatus;

      /* Determine if we are at the final node or an intermediate one */
      uCurrentType = (i == ulDepth) ? uType : TYPE_DIR;

      if (Node_getType(oNCurr) != TYPE_DIR) {
         Path_free(oPPrefix);
         return NOT_A_DIRECTORY;
      }

      if (Node_hasChild(oNCurr, oPPrefix, &ulChildID)) {
         /* Node exists. If it's the final level, it's a duplicate */
         if (i == ulDepth) {
            Path_free(oPPrefix);
            return ALREADY_IN_TREE;
         }
         Node_getChild(oNCurr, ulChildID, &oNCurr);
      } else {
         /* Node is missing, create it */
         if (i == ulDepth) {
            /* Create the final node (file or dir) */
            iStatus = Node_new(oPPrefix, oNCurr, uCurrentType, &oNChild, ulLength, pvContents);
         } else {
            /* Create an intermediate directory */
            iStatus = Node_new(oPPrefix, oNCurr, TYPE_DIR, &oNChild, 0, NULL);
         }

         if (iStatus != SUCCESS) {
            Path_free(oPPrefix);
            return iStatus;
         }
         oNCurr = oNChild;
      }
      Path_free(oPPrefix);
   }

   return SUCCESS;
}

/* PUBLIC API FUNCTIONS BELOW */

int FT_init(void) {
   if (bIsInitialized) return INITIALIZATION_ERROR;
   oNRoot = NULL;
   bIsInitialized = TRUE;
   return SUCCESS;
}

int FT_destroy(void) {
   if (!bIsInitialized) return INITIALIZATION_ERROR;
   if (oNRoot != NULL) {
      Node_free(oNRoot);
      oNRoot = NULL;
   }
   bIsInitialized = FALSE;
   return SUCCESS;
}

int FT_insertDir(const char *pcPath) {
   Path_T oPPath = NULL;
   int iStatus;

   if (!bIsInitialized) return INITIALIZATION_ERROR;

   iStatus = Path_new(pcPath, &oPPath);
   if (iStatus != SUCCESS) return BAD_PATH;

   iStatus = FT_insertNode(oPPath, TYPE_DIR, NULL, 0);
   Path_free(oPPath);
   return iStatus;
}

boolean FT_containsDir(const char *pcPath) {
   Path_T oPPath = NULL;
   Node_T oNNode = NULL;
   int iStatus;

   if (!bIsInitialized) return FALSE;

   iStatus = Path_new(pcPath, &oPPath);
   if (iStatus != SUCCESS) return FALSE;

   iStatus = FT_traversePath(oPPath, &oNNode);
   Path_free(oPPath);

   if (iStatus == SUCCESS && Node_getType(oNNode) == TYPE_DIR) {
      return TRUE;
   }
   return FALSE;
}

int FT_rmDir(const char *pcPath) {
   Path_T oPPath = NULL;
   Node_T oNNode = NULL;
   Node_T oNParent = NULL;
   size_t ulIndex;
   int iStatus;

   if (!bIsInitialized) return INITIALIZATION_ERROR;

   iStatus = Path_new(pcPath, &oPPath);
   if (iStatus != SUCCESS) return BAD_PATH;

   iStatus = FT_traversePath(oPPath, &oNNode);
   if (iStatus != SUCCESS) {
      Path_free(oPPath);
      return iStatus; /* Propagate NO_SUCH_PATH or NOT_A_DIRECTORY */
   }

   if (Node_getType(oNNode) != TYPE_DIR) {
      Path_free(oPPath);
      return NOT_A_DIRECTORY;
   }

   /* Special case: removing the root */
   if (oNNode == oNRoot) {
      Node_free(oNRoot);
      oNRoot = NULL;
      Path_free(oPPath);
      return SUCCESS;
   }

   /* Removing a non-root directory */
   oNParent = Node_getParent(oNNode);
   if (Node_hasChild(oNParent, oPPath, &ulIndex)) {
      /* UNLINK from parent's array before freeing */
      Node_unlinkChild(oNParent, ulIndex); 
      Node_free(oNNode);
   }

   Path_free(oPPath);
   return SUCCESS;
}

int FT_insertFile(const char *pcPath, void *pvContents, size_t ulLength) {
   Path_T oPPath = NULL;
   int iStatus;

   if (!bIsInitialized) return INITIALIZATION_ERROR;

   iStatus = Path_new(pcPath, &oPPath);
   if (iStatus != SUCCESS) return BAD_PATH;

   iStatus = FT_insertNode(oPPath, TYPE_FILE, pvContents, ulLength);
   Path_free(oPPath);
   return iStatus;
}

boolean FT_containsFile(const char *pcPath) {
   Path_T oPPath = NULL;
   Node_T oNNode = NULL;
   int iStatus;

   if (!bIsInitialized) return FALSE;

   iStatus = Path_new(pcPath, &oPPath);
   if (iStatus != SUCCESS) return FALSE;

   iStatus = FT_traversePath(oPPath, &oNNode);
   Path_free(oPPath);

   if (iStatus == SUCCESS && Node_getType(oNNode) == TYPE_FILE) {
      return TRUE;
   }
   return FALSE;
}

int FT_rmFile(const char *pcPath) {
   Path_T oPPath = NULL;
   Node_T oNNode = NULL;
   Node_T oNParent = NULL;
   size_t ulIndex;
   int iStatus;

   if (!bIsInitialized) return INITIALIZATION_ERROR;

   iStatus = Path_new(pcPath, &oPPath);
   if (iStatus != SUCCESS) return BAD_PATH;

   iStatus = FT_traversePath(oPPath, &oNNode);
   if (iStatus != SUCCESS) {
      Path_free(oPPath);
      return iStatus;
   }

   if (Node_getType(oNNode) != TYPE_FILE) {
      Path_free(oPPath);
      return NOT_A_FILE;
   }

   /* A file cannot be the root, so it must have a parent */
   oNParent = Node_getParent(oNNode);
   if (Node_hasChild(oNParent, oPPath, &ulIndex)) {
      /* UNLINK from parent's array before freeing */
      Node_unlinkChild(oNParent, ulIndex); 
      Node_free(oNNode);
   }

   Path_free(oPPath);
   return SUCCESS;
}

void *FT_getFileContents(const char *pcPath) {
   Path_T oPPath = NULL;
   Node_T oNNode = NULL;
   int iStatus;

   if (!bIsInitialized) return NULL;

   iStatus = Path_new(pcPath, &oPPath);
   if (iStatus != SUCCESS) return NULL;

   iStatus = FT_traversePath(oPPath, &oNNode);
   Path_free(oPPath);

   if (iStatus != SUCCESS || Node_getType(oNNode) != TYPE_FILE) {
      return NULL;
   }

   return Node_getContents(oNNode);
}

void *FT_replaceFileContents(const char *pcPath, void *pvNewContents, size_t ulNewLength) {
   Path_T oPPath = NULL;
   Node_T oNNode = NULL;
   int iStatus;

   if (!bIsInitialized) return NULL;

   iStatus = Path_new(pcPath, &oPPath);
   if (iStatus != SUCCESS) return NULL;

   iStatus = FT_traversePath(oPPath, &oNNode);
   Path_free(oPPath);

   if (iStatus != SUCCESS || Node_getType(oNNode) != TYPE_FILE) {
      return NULL;
   }

   return Node_setContents(oNNode, pvNewContents, ulNewLength);
}

int FT_stat(const char *pcPath, boolean *pbIsFile, size_t *pulSize) {
   Path_T oPPath = NULL;
   Node_T oNNode = NULL;
   int iStatus;

   assert(pbIsFile != NULL);
   assert(pulSize != NULL);

   if (!bIsInitialized) return INITIALIZATION_ERROR;

   iStatus = Path_new(pcPath, &oPPath);
   if (iStatus != SUCCESS) return BAD_PATH;

   iStatus = FT_traversePath(oPPath, &oNNode);
   Path_free(oPPath);

   if (iStatus != SUCCESS) return iStatus;

   if (Node_getType(oNNode) == TYPE_FILE) {
      *pbIsFile = TRUE;
      *pulSize = Node_getContentsLength(oNNode);
   } else {
      *pbIsFile = FALSE;
      /* Directories do not change *pulSize per the ft.h specs */
   }

   return SUCCESS;
}

char *FT_toString(void) {
   if (!bIsInitialized) return NULL;
   
   if (oNRoot == NULL) {
      /* Return an empty string if initialized but empty */
      char *pcResult = malloc(1);
      if (pcResult != NULL) pcResult[0] = '\0';
      return pcResult;
   }
   
   return Node_toString(oNRoot);
}