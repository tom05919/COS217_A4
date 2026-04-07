/*--------------------------------------------------------------------*/
/* nodeFT.h                                                           */
/* Author: Ty Lipscomb and Tom Wang                                   */
/*--------------------------------------------------------------------*/

#ifndef NODE_INCLUDED
#define NODE_INCLUDED

#include <stddef.h>
#include "a4def.h"
#include "path.h"


/* A Node_T is a node in a File Tree */
typedef struct node *Node_T;

/* Used to differentiate between directories and files in the same node structure. */
typedef enum {TYPE_DIR, TYPE_FILE} NodeType;

/*
  Built upon the original node logic.
  Can take in either a directory or file node, determined by NodeType.
  If uType is TYPE_DIR, then pvContents and ulLength should be set to NULL (unused).
  If uType is TYPE_FILE, then pvContents and ulLength should be set to the file's contents and length, respectively.
  Both will take in oPPath and oNParent as before.
  Returns SUCCESS if the new node is created successfully.
*/
int Node_new(Path_T oPPath, Node_T oNParent, NodeType uType, Node_T *poNResult, size_t ulLength, void *pvContents);

/* Internal helper function - finds a child node */
boolean Node_find(Node_T oNParent, Path_T oPPath, NodeType uType, size_t *pulIndex);

/* new functions defined before */
/* Returns the NodeType of a provided node (oNNode)*/
NodeType Node_getType(Node_T oNNode);

/* Returns a pointer to the contents of a provided file node (oNNode) MUST BE FILE TYPE OTHERWISE RETURN ERROR */
void *Node_getContents(Node_T oNNode);

/* returns file length. Cannot call on directory nodes otherwise logic error*/
size_t Node_getContentsLength(Node_T oNNode);

/* updates the file contents. Returns the old contents on success, ERROR on failure*/
void *Node_setContents(Node_T oNNode, void *pvNewContents, size_t ulNewLength);

/*
  Destroys and frees all memory allocated for the subtree rooted at
  oNNode, i.e., deletes this node and all its descendents. Returns the
  number of nodes deleted.
*/
size_t Node_free(Node_T oNNode);

/* Returns the path object representing oNNode's absolute path. */
Path_T Node_getPath(Node_T oNNode);

/*
  Returns TRUE if oNParent has a child with path oPPath. Returns
  FALSE if it does not.

  If oNParent has such a child, stores in *pulChildID the child's
  identifier (as used in Node_getChild). If oNParent does not have
  such a child, stores in *pulChildID the identifier that such a
  child _would_ have if inserted.
*/
boolean Node_hasChild(Node_T oNParent, Path_T oPPath,
                         size_t *pulChildID);

/* Internal helper function - adds a child node */
int Node_addChild(Node_T oNParent, Node_T oNChild, size_t ulIndex);

/* Returns the number of children that oNParent has. */
size_t Node_getNumChildren(Node_T oNParent);

/*
  Returns an int SUCCESS status and sets *poNResult to be the child
  node of oNParent with identifier ulChildID, if one exists.
  Otherwise, sets *poNResult to NULL and returns status:
  * NO_SUCH_PATH if ulChildID is not a valid child for oNParent
*/
int Node_getChild(Node_T oNParent, size_t ulChildID,
                  Node_T *poNResult);

/* Remove (unlink) the child at ulChildID from oNParent's children array.
   Caller is responsible for freeing the node. */
void Node_unlinkChild(Node_T oNParent, size_t ulChildID);

/*
  Returns a the parent node of oNNode.
  Returns NULL if oNNode is the root and thus has no parent.
*/
Node_T Node_getParent(Node_T oNNode);


/* add comments*/
int Node_compare(Node_T oNFirst, Node_T oNSecond);

/*
  Compares oNFirst and oNSecond lexicographically based on their paths.
  Returns <0, 0, or >0 if onFirst is "less than", "equal to", or
  "greater than" oNSecond, respectively.
*/

/*
  Returns a string representation for oNNode, or NULL if
  there is an allocation error.

  Allocates memory for the returned string, which is then owned by
  the caller!
*/
char *Node_toString(Node_T oNNode);

#endif
