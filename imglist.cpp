/**
 *  @file        imglist.cpp
 *  @description Contains partial implementation of ImgList class
 *               for CPSC 221 PA1
 *               Function bodies to be completed by student
 * 
 *  THIS FILE WILL BE SUBMITTED
 */

#include "imglist.h"

#include <math.h> // provides fmax, fmin, and fabs functions
#include <limits>



/*********************
* CONSTRUCTORS, ETC. *
*********************/

/**
 * Default constructor. Makes an empty list
 */
ImgList::ImgList() {
    // set appropriate values for all member attributes here
    northwest = nullptr;
    southeast = nullptr;
	
}

/**
 * Creates a list from image data
 * @pre img has dimensions of at least 1x1
 */
ImgList::ImgList(PNG& img) {
    // build the linked node structure and set the member attributes appropriately
    northwest = nullptr;
    southeast = nullptr;
    unsigned int width = img.width();
    unsigned int height = img.height();

    // Array to keep track of the nodes, for easily setting the north and south links.
    std::vector<std::vector<ImgNode*>> nodes(height, std::vector<ImgNode*>(width, nullptr));

    for (unsigned int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            // Create a new node
            ImgNode* newNode = new ImgNode();
            newNode->colour = *img.getPixel(x, y);  // Assuming RGBAPixel is the colour type
            newNode->north = (y > 0) ? nodes[y - 1][x] : nullptr;
            newNode->south = nullptr;  // Will be set in the next iteration of y
            newNode->west = (x > 0) ? nodes[y][x - 1] : nullptr;
            newNode->east = nullptr;   // Will be set in the next iteration of x

            // Set the south link of the northern neighbor
            if (newNode->north) {
                newNode->north->south = newNode;
            }

            // Set the east link of the western neighbor
            if (newNode->west) {
                newNode->west->east = newNode;
            }

            // Store the pointer to the node
            nodes[y][x] = newNode;

            // Set the northwest and southeast pointers
            if (x == 0 && y == 0) {
                northwest = newNode;
            }
            if (x == width - 1 && y == height - 1) {
                southeast = newNode;
            }
        }
    }

    // Link the last column's east pointers and the last row's south pointers
    for (unsigned int y = 0; y < height; y++) {
        nodes[y][width - 1]->east = nullptr;
    }
    for (unsigned int x = 0; x < width; x++) {
        nodes[height - 1][x]->south = nullptr;
    }

	
}

/************
* ACCESSORS *
************/

/**
 * Returns the horizontal dimension of this list (counted in nodes)
 * Note that every row will contain the same number of nodes, whether or not
 *   the list has been carved.
 * We expect your solution to take linear time in the number of nodes in the
 *   x dimension.
 */
unsigned int ImgList::GetDimensionX() const {
    // replace the following line with your implementation
    unsigned int count = 0;
    ImgNode* currentNode = northwest;
    while (currentNode != nullptr) {
        count++;
        currentNode = currentNode->east;
    }
    return count;
}

/**
 * Returns the vertical dimension of the list (counted in nodes)
 * It is useful to know/assume that the grid will never have nodes removed
 *   from the first or last columns. The returned value will thus correspond
 *   to the height of the PNG image from which this list was constructed.
 * We expect your solution to take linear time in the number of nodes in the
 *   y dimension.
 */
unsigned int ImgList::GetDimensionY() const {
    // replace the following line with your implementation
    unsigned int count = 0;
    ImgNode* currentNode = northwest;
    while (currentNode != nullptr) {
        count++;
        currentNode = currentNode->south;
    }
    return count;
}

/**
 * Returns the horizontal dimension of the list (counted in original pixels, pre-carving)
 * The returned value will thus correspond to the width of the PNG image from
 *   which this list was constructed.
 * We expect your solution to take linear time in the number of nodes in the
 *   x dimension.
 */
unsigned int ImgList::GetDimensionFullX() const {
    unsigned int count = 0;
    ImgNode* currentNode = northwest;
    while (currentNode != nullptr) {
        count += 1 + currentNode->skipright;  // count the node itself and any skipped nodes
        currentNode = currentNode->east;
    }
    return count;
}

/**
 * Returns a pointer to the node which best satisfies the specified selection criteria.
 * The first and last nodes in the row cannot be returned.
 * @pre rowstart points to a row with at least 3 physical nodes
 * @pre selectionmode is an integer in the range [0,1]
 * @param rowstart - pointer to the first node in a row
 * @param selectionmode - criterion used for choosing the node to return
 *          0: minimum "brightness" across row, not including extreme left or right nodes
 *          1: node with minimum total of "colour difference" with its left neighbour and with its right neighbour.
 *        In the (likely) case of multiple candidates that best match the criterion,
 *        the left-most node satisfying the criterion (excluding the row's starting node)
 *        will be returned.
 * A note about "brightness" and "colour difference":
 * For PA1, "brightness" will be the sum over the RGB colour channels, multiplied by alpha.
 * "colour difference" between two pixels can be determined
 * using the "distanceTo" function found in RGBAPixel.h.
 */
ImgNode* ImgList::SelectNode(ImgNode* rowstart, int selectionmode) {
    if (rowstart == nullptr || rowstart->east == nullptr || rowstart->east->east == nullptr) {
        return nullptr; // Ensures there are at least three nodes.
    }

    ImgNode* currentNode = rowstart->east; // Start with the second node to ensure the first and last nodes are not selected.
    ImgNode* bestNode = nullptr;
    double bestValue = std::numeric_limits<double>::max();

    while (currentNode != nullptr && currentNode->east != nullptr) { // Ensure not to consider the last node.
        double currentValue = 0.0;

        if (selectionmode == 0) {
            // Compute brightness as the sum of the RGB values, multiplied by alpha (normalized to 0-1).
            currentValue = (currentNode->colour.r + currentNode->colour.g + currentNode->colour.b) * (currentNode->colour.a / 255.0);
        } else if (selectionmode == 1) {
            // Compute color difference using the distanceTo method provided by RGBAPixel.
            // Ensure left and right neighbors exist before attempting to access them.
            if (currentNode->west != nullptr && currentNode->east != nullptr) {
                double distanceToLeft = currentNode->colour.distanceTo(currentNode->west->colour);
                double distanceToRight = currentNode->colour.distanceTo(currentNode->east->colour);
                currentValue = distanceToLeft + distanceToRight;
            }
        }

        // Update bestNode if the current node's value is better (lower) than the bestValue found so far.
        if (currentValue < bestValue) {
            bestValue = currentValue;
            bestNode = currentNode;
        }

        currentNode = currentNode->east; // Move to the next node in the row.
    }

    return bestNode;
}

/**
 * Renders this list's pixel data to a PNG, with or without filling gaps caused by carving.
 * @pre fillmode is an integer in the range of [0,2]
 * @param fillgaps - whether or not to fill gaps caused by carving
 *          false: render one pixel per node, ignores fillmode
 *          true: render the full width of the original image,
 *                filling in missing nodes using fillmode
 * @param fillmode - specifies how to fill gaps
 *          0: solid, uses the same colour as the node at the left of the gap
 *          1: solid, using the averaged values (all channels) of the nodes at the left and right of the gap
 *          2: linear gradient between the colour (all channels) of the nodes at the left and right of the gap
 *             e.g. a gap of width 1 will be coloured with 1/2 of the difference between the left and right nodes
 *             a gap of width 2 will be coloured with 1/3 and 2/3 of the difference
 *             a gap of width 3 will be coloured with 1/4, 2/4, 3/4 of the difference, etc.
 *             Like fillmode 1, use the smaller difference interval for hue,
 *             and the smaller-valued average for diametric hues
 */
PNG ImgList::Render(bool fillgaps, int fillmode) const {
    // // Add/complete your implementation below
  
    PNG outpng; // This will be returned later. Might be a good idea to resize it at some point.
    int width, height;

    // Determine dimensions based on whether gaps should be filled
    if (!fillgaps) {
        width = GetDimensionX();
        height = GetDimensionY();
    } else {
        width = GetDimensionFullX();
        height = GetDimensionY();
    }

    outpng.resize(width, height); // Resize the output image accordingly

    ImgNode* currRow = northwest;
    for (int y = 0; y < height; y++) {
        ImgNode* curr = currRow;
        for (int x = 0; curr != nullptr && x < width; x++) {
            RGBAPixel* currPixel = outpng.getPixel(x, y);

            if (fillgaps && curr->east != nullptr && curr->skipright > 0) {
                // Handle gap filling based on fillmode
                for (unsigned int gap = 0; gap <= curr->skipright && x < width; gap++) {
                    if (fillmode == 0 || gap == 0) { // Mode 0 or first pixel in gap
                        *currPixel = curr->colour;
                    } else if (fillmode == 1) { // Mode 1
                        // Average color calculation
                        currPixel->r = (curr->colour.r + curr->east->colour.r) / 2;
                        currPixel->g = (curr->colour.g + curr->east->colour.g) / 2;
                        currPixel->b = (curr->colour.b + curr->east->colour.b) / 2;
                        currPixel->a = (curr->colour.a + curr->east->colour.a) / 2;
                    } else if (fillmode == 2) { // Mode 2
                        // Gradient color calculation
                        double fraction = double(gap) / (curr->skipright + 1);
                        currPixel->r = curr->colour.r + fraction * (curr->east->colour.r - curr->colour.r);
                        currPixel->g = curr->colour.g + fraction * (curr->east->colour.g - curr->colour.g);
                        currPixel->b = curr->colour.b + fraction * (curr->east->colour.b - curr->colour.b);
                        currPixel->a = curr->colour.a + fraction * (curr->east->colour.a - curr->colour.a);
                    }

                    if (gap < curr->skipright) { // Move to next pixel if within a gap
                        currPixel = outpng.getPixel(++x, y);
                    }
                }
                curr = curr->east; // Move past the gap
            } else { // No gap or not filling gaps
                *currPixel = curr->colour;
                curr = curr->east;
            }
        }
        currRow = currRow->south; // Move to the next row
    }

    return outpng;
}

/************
* MODIFIERS *
************/

/**
 * Removes exactly one node from each row in this list, according to specified criteria.
 * The first and last nodes in any row cannot be carved.
 * @pre this list has at least 3 nodes in each row
 * @pre selectionmode is an integer in the range [0,1]
 * @param selectionmode - see the documentation for the SelectNode function.
 * @param this list has had one node removed from each row. Neighbours of the created
 *       gaps are linked appropriately, and their skip values are updated to reflect
 *       the size of the gap.
 */
void ImgList::Carve(int selectionmode) {
    // add your implementation here
    if (northwest == nullptr) return; // Empty list check

    ImgNode* currentRowStart = northwest;
    while (currentRowStart != nullptr) {
        ImgNode* nodeToRemove = SelectNode(currentRowStart, selectionmode);
        if (nodeToRemove != nullptr && nodeToRemove->east != nullptr && nodeToRemove->west != nullptr) {
            // Update the east pointer of the western neighbor
            nodeToRemove->west->east = nodeToRemove->east;
            // Update the west pointer of the eastern neighbor
            nodeToRemove->east->west = nodeToRemove->west;
            // Update skip values if needed
            nodeToRemove->west->skipright += 1 + nodeToRemove->skipright;
            nodeToRemove->east->skipleft += 1 + nodeToRemove->skipleft;

            // Finally, delete the node
            delete nodeToRemove;
        }
        // Move to the first node of the next row
        currentRowStart = currentRowStart->south;
    }
	
}

// note that a node on the boundary will never be selected for removal
/**
 * Removes "rounds" number of nodes (up to a maximum of node width - 2) from each row,
 * based on specific selection criteria.
 * Note that this should remove one node from every row, repeated "rounds" times,
 * and NOT remove "rounds" nodes from one row before processing the next row.
 * @pre selectionmode is an integer in the range [0,1]
 * @param rounds - number of nodes to remove from each row
 *        If rounds exceeds node width - 2, then remove only node width - 2 nodes from each row.
 *        i.e. Ensure that the final list has at least two nodes in each row.
 * @post this list has had "rounds" nodes removed from each row. Neighbours of the created
 *       gaps are linked appropriately, and their skip values are updated to reflect
 *       the size of the gap.
 */
void ImgList::Carve(unsigned int rounds, int selectionmode) {
    // add your implementation here
    if (northwest == nullptr) return; // Check for an empty list.

    // Calculate the max rounds possible to ensure at least two nodes remain in each row.
    unsigned int maxRounds = GetDimensionX() - 2;
    rounds = std::min(rounds, maxRounds); // Ensure 'rounds' does not exceed 'maxRounds'.

    for (unsigned int round = 0; round < rounds; ++round) {
        ImgNode* currentRowStart = northwest;
        while (currentRowStart != nullptr) {
            ImgNode* nodeToRemove = SelectNode(currentRowStart, selectionmode);

            if (nodeToRemove != nullptr) {
                // Before removal, update skip values.
                if (nodeToRemove->west != nullptr) {
                    nodeToRemove->west->east = nodeToRemove->east;
                    nodeToRemove->west->skipright += 1 + nodeToRemove->skipright; // Update skip value.
                }
                if (nodeToRemove->east != nullptr) {
                    nodeToRemove->east->west = nodeToRemove->west;
                    nodeToRemove->east->skipleft += 1 + nodeToRemove->skipleft; // Update skip value.
                }

                // Now, remove the node.
                delete nodeToRemove;
            }

            // Move to the start of the next row for the current round of removal.
            currentRowStart = (currentRowStart->south != nullptr && currentRowStart->south->west != nullptr) ? currentRowStart->south->west : currentRowStart->south;
        }
    }
	
}


/*
 * Helper function deallocates all heap memory associated with this list,
 * puts this list into an "empty" state. Don't forget to set your member attributes!
 * @post this list has no currently allocated nor leaking heap memory,
 *       member attributes have values consistent with an empty list.
 */
void ImgList::Clear() {
    // add your implementation here
     ImgNode* currentRow = northwest;
    while (currentRow != nullptr) {
        ImgNode* currentNode = currentRow;
        currentRow = currentRow->south; // Move to the next row
        
        while (currentNode != nullptr) {
            ImgNode* temp = currentNode;
            currentNode = currentNode->east; // Move to the next node in the row
            delete temp; // Deallocate the node
        }
    }
    // Reset member attributes
    northwest = nullptr;
    southeast = nullptr;
	
}

/**
 * Helper function copies the contents of otherlist and sets this list's attributes appropriately
 * @pre this list is empty
 * @param otherlist - list whose contents will be copied
 * @post this list has contents copied from by physically separate from otherlist
 */
void ImgList::Copy(const ImgList& otherlist) {
    // add your implementation here
     if (otherlist.northwest == nullptr) {
        northwest = nullptr;
        southeast = nullptr;
        return; // Other list is empty, so this one will be too
    }

    // Initialize node grid to store the pointers to new nodes
    unsigned int height = otherlist.GetDimensionY();
    unsigned int width = otherlist.GetDimensionX();
    std::vector<std::vector<ImgNode*>> newNodes(height, std::vector<ImgNode*>(width, nullptr));

    // Copy the nodes row by row
    ImgNode* rowNode = otherlist.northwest;
    for (unsigned int y = 0; y < height; y++) {
        ImgNode* colNode = rowNode;
        for (unsigned int x = 0; x < width; x++) {
            ImgNode* newNode = new ImgNode(*colNode); // Use the copy constructor of ImgNode
            newNodes[y][x] = newNode;

            if (y > 0) {
                newNode->north = newNodes[y - 1][x];
                newNode->north->south = newNode;
            }
            if (x > 0) {
                newNode->west = newNodes[y][x - 1];
                newNode->west->east = newNode;
            }
            if (x == 0 && y == 0) {
                northwest = newNode; // Set the northwest pointer
            }
            if (x == width - 1 && y == height - 1) {
                southeast = newNode; // Set the southeast pointer
            }
            colNode = colNode->east; // Move to the next node in the row
        }
        rowNode = rowNode->south; // Move to the next row
    }
	
}

/*************************************************************************************************
* IF YOU DEFINED YOUR OWN PRIVATE FUNCTIONS IN imglist-private.h, YOU MAY ADD YOUR IMPLEMENTATIONS BELOW *
*************************************************************************************************/

