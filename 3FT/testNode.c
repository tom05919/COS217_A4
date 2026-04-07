#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "nodeFT.h"
#include "path.h"

int main(void) {
    Node_T root, file1, dir1, result;
    Path_T pRoot, pFile1, pDir1;
    size_t index;
    int status;

    printf("Starting Node Stress Test...\n");

    /* 1. Test Root Creation */
    pRoot = Path_new("/");
    status = Node_new(pRoot, NULL, TYPE_DIR, &root, 0, NULL);
    assert(status == SUCCESS);
    printf("Created Root.\n");

    /* 2. Test File vs Directory Sorting (The Zones) */
    pDir1 = Path_new("/myDir");
    pFile1 = Path_new("/myFile");
    
    /* Add a Directory first */
    Node_new(pDir1, root, TYPE_DIR, &dir1, 0, NULL);
    /* Add a File second */
    Node_new(pFile1, root, TYPE_FILE, &file1, 5, "hello");

    /* 3. Verify Sorting: File (myFile) should be at index 0, Dir (myDir) at index 1 */
    assert(Node_getNumChildren(root) == 2);
    Node_getChild(root, 0, &result);
    assert(Node_getType(result) == TYPE_FILE); // Success if Files come first!
    printf("Sorting order verified: Files before Directories.\n");

    /* 4. Test toString */
    char *treeStr = Node_toString(root);
    printf("\nTree Structure:\n%s\n", treeStr);
    free(treeStr);

    /* 5. Cleanup */
    size_t deleted = Node_free(root);
    printf("\nDeleted %zu nodes total.\n", deleted);

    printf("Test passed successfully!\n");
    return 0;
}