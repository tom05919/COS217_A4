#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "nodeFT.h"
#include "path.h"
#include "a4def.h"

int main(void) {
   /* C90: All variables must be declared at the top of the function */
   Node_T oRoot, oDir1, oFile1, oTemp;
   Path_T pRoot, pDir1, pFile1, pFile2;
   char *pcTreeStr;
   void *pvOldContents;
   size_t ulIndex, ulCount;
   int iStatus;

   printf("--- Starting Comprehensive NodeFT Test ---\n");

   /* 1. TEST: Root Creation */
   assert(Path_new("root", &pRoot) == SUCCESS);
   iStatus = Node_new(pRoot, NULL, TYPE_DIR, &oRoot, 0, NULL);
   assert(iStatus == SUCCESS);
   assert(Node_getType(oRoot) == TYPE_DIR);
   assert(Node_getParent(oRoot) == NULL);
   printf("Step 1: Root directory created successfully.\n");

   /* 2. TEST: Directory and File Creation */
   assert(Path_new("root/dir1", &pDir1) == SUCCESS);
   assert(Path_new("root/file1", &pFile1) == SUCCESS);
   
   /* We add the Directory first, then the File */
   iStatus = Node_new(pDir1, oRoot, TYPE_DIR, &oDir1, 0, NULL);
   assert(iStatus == SUCCESS);
   iStatus = Node_new(pFile1, oRoot, TYPE_FILE, &oFile1, 6, "Hello");
   assert(iStatus == SUCCESS);
   printf("Step 2: Sub-nodes created.\n");

   /* 3. TEST: Sorting (Files before Directories) */
   /* Even though we added dir1 FIRST, file1 should be at index 0 
      because of our Node_compare logic. */
   assert(Node_getNumChildren(oRoot) == 2);
   Node_getChild(oRoot, 0, &oTemp);
   assert(Node_getType(oTemp) == TYPE_FILE); 
   Node_getChild(oRoot, 1, &oTemp);
   assert(Node_getType(oTemp) == TYPE_DIR);
   printf("Step 3: 'Files before Directories' sorting verified.\n");

   /* 4. TEST: File Content Accessors */
   assert(Node_getContentsLength(oFile1) == 6);
   assert(strcmp((char*)Node_getContents(oFile1), "Hello") == 0);
   
   pvOldContents = Node_setContents(oFile1, "World!!", 8);
   assert(strcmp((char*)pvOldContents, "Hello") == 0);
   assert(Node_getContentsLength(oFile1) == 8);
   free(pvOldContents); /* setContents returns old memory; we must free it */
   printf("Step 4: File content getters/setters verified.\n");

   /* 5. TEST: Search (Two-Zone logic) */
   /* Search for the file */
   assert(Node_hasChild(oRoot, pFile1, &ulIndex) == TRUE);
   assert(ulIndex == 0);
   /* Search for the directory */
   assert(Node_hasChild(oRoot, pDir1, &ulIndex) == TRUE);
   assert(ulIndex == 1);
   /* Search for something non-existent */
   assert(Path_new("root/ghost", &pFile2) == SUCCESS);
   assert(Node_hasChild(oRoot, pFile2, &ulIndex) == FALSE);
   printf("Step 5: Two-zone binary search verified.\n");

   /* 6. TEST: String Representation */
   pcTreeStr = Node_toString(oRoot);
   assert(pcTreeStr != NULL);
   printf("\nStep 6: toString Output:\n%s\n", pcTreeStr);
   free(pcTreeStr);

   /* 7. TEST: Conflicting Path Error */
   /* Try to add a child to a file (oFile1) */
   assert(Path_new("root/file1/bad", &pFile2) == SUCCESS);
   iStatus = Node_new(pFile2, oFile1, TYPE_FILE, &oTemp, 0, NULL);
   assert(iStatus == NOT_A_DIRECTORY);
   printf("Step 7: Error handling (NOT_A_DIRECTORY) verified.\n");

   /* 8. TEST: Recursive Free */
   ulCount = Node_free(oRoot);
   /* Root + Dir1 + File1 = 3 nodes */
   printf("Step 8: Deleted %lu nodes.\n", (unsigned long)ulCount);
   assert(ulCount == 3);

   printf("\n--- ALL TESTS PASSED SUCCESSFULLY ---\n");
   
   /* Cleanup leftover paths */
   Path_free(pRoot);
   Path_free(pDir1);
   Path_free(pFile1);
   Path_free(pFile2);

   return 0;
}