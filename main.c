#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define fastVectorLen 2048
int blockCount = 0;
int wordSize;
int usableBlocks;
int j = 0, maxBlockCount;


/**
 * this program implements an optimized version of the game wordle; the data structure is based on a binary search tree; each node is made of a struct "word".
 */
struct word *treeRoot = NULL ;

/**
 * this struct represents a node of the bst containing all words needed for the game;
 * the short int "usable" is used as flag for whether the word is still valid for a match
 */
struct word{
    struct word *left;
    struct word *right;
    short int usable;
    char word[10];
};

/**
 * this vector is used when the number of valid word in the match is lower than it's size; it is implemented as it is way faster
 * in cases of small games or of players which tend to not use hints; in such case the program would navigate the whole bst even if not needed, and in
 * case of high-word counts this would be expensive
 */
struct word* fastVector[fastVectorLen];

//flag for program
int fastVectorUsage = 0;

/**
 * data structure used to keep information about the characters during a match
 */
struct map{
    char c;
    int count;
    int permaCount;
    int mode;    //mode 0 -> non-present in the secret word; mode 1 -> non-max ; mode 2 -> max num
};

struct map map[64];

/**
 * this procedure receives a word pointer and writes to output both such words and all beneath it in the bst
 * @param current the node from which to start
 */
void print(struct word *current){
    struct word *temp;

    while(current != NULL){
        if(current->left == NULL){
            if(current->usable == 1){
                puts(current->word);
            }
            current = current->right;
        }
        else{
            temp = current->left;
            while(temp->right != NULL && temp->right != current){
                temp = temp->right;
            }

            if(temp->right == NULL){
                temp->right = current;
                current = current->left;
            }
            else{
                if(current->usable == 1){
                    puts(current->word);
                }
                temp->right = NULL;
                current = current->right;
            }
        }
    }
}

/**
 * this function resets the value of the flag "usable" in each word beneath the word whose pointer is passed (included the passed word)
 * @param node
 */
void resetSubR(struct word *node){
    if(node->left != NULL) resetSubR(node->left);
    node->usable = 1;
    if(node->right != NULL) resetSubR(node->right);
}
/**
 * utility function returning 1 if @param searched is contained in @param vector, 0 otherwise
 * @param vector vector to find the searched value in
 * @param searched value to search in vector
 * @return 1 if found, 0 otherwise
 */
int presentInVector(int vector[], int searched){
    register int i;
    for(i = 0; i < wordSize; i++){
        if(vector[i] == searched){
            return 1;
        }
    }
    return 0;
}


/**
 * this function checks whether a player guess is present
 * @param word
 * @return 0 if not present, 1 if present
 */
int present(char *word){
    struct word *current = treeRoot;

    while (current != NULL) {
        if(strcmp(word, current->word) == 0) return 1;
        if (strcmp(word, current->word) < 0) {
            current = current->left;
        } else current = current->right;
    }
    return 0;
}


/**
 * used to map from ASCII value to the position in map of a char
 * @param c char on which you are computing
 * @return int value mapping @param c to a position in map
 */
int mapFunction(char c){
    if(c==45) return 0; //"-" symbol
    else if(c == 95) return 1; //"_" symbol
    else if(c >= 48 && c <=57) return c-46; //numbers
    else if(c >=65 && c <=90) return c-53; //upper case letters
    else if(c >=97 && c <=122) return c-59; //lower case letters
    else return 90;
}
/**
 * reverse of @function mapFunction
 * @param i position in map
 * @return corresponding character
 */
int reverseMapFunction(int i){
    if(i == 0) return 45;
    else if(i == 1) return 95;
    else if(i >= 2 && i <= 11) return i + 46;
    else if(i >= 12 && i <= 37) return i + 53;
    else if(i >= 38 && i <= 63) return i + 59;
    else return 0;
}


/**
 * function used to put words in the vector for faster access when needed
 * @param node the node of the tree from which to start
 */
void fastLaneStock(struct word *node){
    if(node->left != NULL) fastLaneStock(node->left);
    if(node->usable == 1){
        fastVector[fastVectorUsage] = node;
        fastVectorUsage++;
    }
    if(node->right != NULL) fastLaneStock(node->right);
}
void fastLaneCheck(char comparison[],char wordTested[],int side[],int noMap[][wordSize]){
    register int i = 0,ii,jj,k = 0,count = 0,currentPointer = 0;
    int sideRef[wordSize];
    struct word *current;
    char c;


    for(i = 0; i < wordSize; i++) {                                 //mode 0 ->non present ; mode 1 -> non max ; mode 2 -> max num
        ii = mapFunction(wordTested[i]);
        if(map[ii].c == '\0') {
            map[ii].c = wordTested[i];
            sideRef[k] = ii;
            k++;
        }

        if(comparison[i] == '|' || comparison[i] == '+'){
            if(map[ii].mode == 0) map[ii].mode = 2;
            else if(map[ii].mode != 2){
                map[ii].mode = 1;
                map[ii].count++;
                if(map[ii].count > map[ii].permaCount) map[ii].permaCount = map[ii].count;
            }
            if(map[ii].mode == 2){
                map[ii].count++;
                if(map[ii].count > map[ii].permaCount) map[ii].permaCount = map[ii].count;
            }
            if(comparison[i] == '|') noMap[ii][i] = 1;
            else{
                for(jj = 0; jj < 64; jj++){
                    if(jj != ii) noMap[jj][i] = 1;
                }
            }
        }
        else if(comparison[i] =='/'){
            if(map[ii].mode == 3) map[ii].mode = 0;
            else if(map[ii].mode == 1) map[ii].mode = 2;
        }
    }
    if(k != wordSize)sideRef[k] = 90;

    for(i = 0; i < k; i++){
        if(map[sideRef[i]].mode == 0){
            for(jj = 0; jj < wordSize; jj++) noMap[sideRef[i]][jj] = 1;
        }
        else if(map[sideRef[i]].mode == 2){
            for(jj = 0; jj < wordSize; jj++){
                if(wordTested[jj] == map[sideRef[i]].c && comparison[jj] != '+') noMap[sideRef[i]][jj] = 1;
            }
        }
    }

    i = 0;
    while( i < wordSize && sideRef[i] != 90 && j < wordSize){
        if((map[sideRef[i]].mode == 1 || map[sideRef[i]].mode == 2) && presentInVector(side, sideRef[i]) == 0 ){
            side[j] = sideRef[i];
            j++;
        }
        i++;
    }

    while(currentPointer < fastVectorUsage){
        current = fastVector[currentPointer];

        if(current->usable == 1){
            i = 0;
            ii = 0;

            while(current->usable == 1 && ii < wordSize){
                if(comparison[ii] == '+' && current->word[ii] != wordTested[ii]){
                    current->usable = 0;
                    usableBlocks--;
                }
                ii++;
            }

            while(current->usable == 1 && i < k){
                ii = 0;

                switch(map[sideRef[i]].mode){
                    case 0:
                        c = map[sideRef[i]].c;
                        while(current->usable == 1 && ii < wordSize){
                            if(current->word[ii] == c){
                                current->usable = 0;
                                usableBlocks--;
                            }
                            ii++;
                        }
                        break;
                    case 1:
                        c = map[sideRef[i]].c;
                        while (current->usable == 1 && ii < wordSize) {
                            if (current->word[ii] == c) {
                                if (comparison[ii] == '|' &&  current->word[ii] == wordTested[ii]){
                                    current->usable = 0;
                                    usableBlocks--;
                                }
                                else count++;
                            }
                            ii++;
                        }
                        if (current->usable == 1 && count < map[sideRef[i]].permaCount){
                            current->usable = 0;
                            usableBlocks--;
                        }
                        count = 0;
                        break;
                    case 2:
                        c = map[sideRef[i]].c;
                        while(current->usable == 1 && ii < wordSize) {
                            if (current->word[ii] == c) {
                                if (comparison[ii] == '|' && current->word[ii] == wordTested[ii]){
                                    current->usable = 0;
                                    usableBlocks--;
                                }
                                else if (comparison[ii] == '/' && current->word[ii] == wordTested[ii]) {
                                    current->usable = 0;
                                    usableBlocks--;
                                }
                                else count++;
                            }
                            ii++;
                        }
                        if (current->usable == 1 && count != map[sideRef[i]].permaCount) {
                            current->usable = 0;
                            usableBlocks--;
                        }
                        count = 0;
                        break;
                }
                i++;
            }
        }
        currentPointer++;
    }
    i = 0;
    while(i < k){
        map[sideRef[i]].c = '\0';
        map[sideRef[i]].count = 0;
        i++;
    }
}


/**
 * this function checks, after the computation made by "wordCheck" which words are still usable after the given hint
 * @param comparison result of computation from wordCheck
 * @param wordTested user's guess
 * @param side vector containing letters present in the right word guessed correctly from user
 * @param noMap matrix[i][ii] containing 0 or 1 whether a letter (i, following mapFucntion numbering) should be in position ii
 */
void usableCheck(char comparison[], char wordTested[], int side[], int noMap[][wordSize]){
    struct word *current = treeRoot, *temp;
    register int i,ii,k = 0,jj;
    int sideRef[wordSize], count = 0;
    char c;

    //here the map (which tracks the number of instances of a letter is updated with the info from the hint)
    //check in case the tree is empty (current is initialized with treeRoot value)
    if(current != NULL){
        for(i = 0; i < wordSize; i++) {                                 //mode 0 -> non-present in the secret word; mode 1 -> non-max ; mode 2 -> max num
            ii = mapFunction(wordTested[i]);

            //if a map cell hasn't been used it's initialized with the corresponding letter
            if(map[ii].c == '\0') {
                map[ii].c = wordTested[i];
                //side ref has the letters
                sideRef[k] = ii;
                k++;
            }

            //this part sets the mode for the letters used in the word the user tried
            if(comparison[i] == '|' || comparison[i] == '+'){
                //as this step is done from left to right we may have set to mode 0 a letter (when finding a '/')  but then found a match with '+'
                if(map[ii].mode == 0) map[ii].mode = 2;
                else if(map[ii].mode != 2){
                    map[ii].mode = 1;
                    map[ii].count++;
                    if(map[ii].count > map[ii].permaCount) map[ii].permaCount = map[ii].count;
                }
                //permacount is increased and then fixed when mode becomes 2
                if(map[ii].mode == 2){
                    map[ii].count++;
                    if(map[ii].count > map[ii].permaCount) map[ii].permaCount = map[ii].count;
                }
                //putting in noMap 1 if a letter shouldn't be in a certain position in the word to guess (with the info I got from the hint)
                if(comparison[i] == '|') noMap[ii][i] = 1;
                    //the other case is '+' which means that we know that in that position there can be only a letter, so all other letters cannot be in position i
                else{
                    for(jj = 0; jj < 64; jj++){
                        if(jj != ii) noMap[jj][i] = 1;
                    }
                }
            }
            else if(comparison[i] =='/'){
                //if still with initialization value mode is set to 0 (ie not present in word)
                if(map[ii].mode == 3) map[ii].mode = 0;
                    //if mode is 1 it means that the max number of occurrences of a certain letter was found; such letter is put in mode 2
                else if(map[ii].mode == 1) map[ii].mode = 2;
            }
        }
        //fill the first empty cell of sideRef with 90 (placeholder value)
        if(k != wordSize) sideRef[k] = 90;

        //for the number of words in sideRef
        for(i = 0; i < k; i++){
            //if the map says that the letter is not in the secret word I put 1 in the corresponding noMap place
            if(map[sideRef[i]].mode == 0){
                for(jj = 0; jj < wordSize; jj++) noMap[sideRef[i]][jj] = 1;
            }
                //if the map says that the letter has reached the max num of instances I put 1 in the corresponding noMap place
            else if(map[sideRef[i]].mode == 2){
                for(jj = 0; jj < wordSize; jj++){
                    if(wordTested[jj] == map[sideRef[i]].c && comparison[jj] != '+') noMap[sideRef[i]][jj] = 1;
                }
            }
        }

        //stocking side vector (contains letters that are discovered to be contained in the right word)
        i = 0;
        while(i < wordSize && sideRef[i] != 90 && j < wordSize){
            if((map[sideRef[i]].mode == 1 || map[sideRef[i]].mode == 2) && presentInVector(side, sideRef[i]) == 0 ){
                side[j] = sideRef[i];
                j++;
            }
            i++;
        }

        //in order crossing of word tree
        while(current != NULL){
            if(current->left == NULL){
                //checking usable flag wherever possible to cut on processing time
                if(current->usable == 1){
                    i = 0;
                    ii = 0;

                    //check the observed word for cases of exact matches '+'
                    while(current->usable == 1 && ii < wordSize){
                        if(comparison[ii] == '+' && current->word[ii] != wordTested[ii]){
                            current->usable = 0;
                            usableBlocks--;
                        }
                        ii++;
                    }

                    //i must be minor than k because we need to check only for the number of letters contained in sideref
                    //we check letter per letter (from sideref) every usable word in the tree
                    while(current->usable == 1 && i < k){
                        ii= 0;

                        //depending on the mode of the observed letter we scan the observed word
                        switch(map[sideRef[i]].mode){

                            //case 0 means that the letter doesn't exist in the word; any word containing it is no longer usable
                            case 0:
                                c = map[sideRef[i]].c;
                                while(current->usable == 1 && ii < wordSize){
                                    if(current->word[ii] == c){
                                        current->usable = 0;
                                        usableBlocks--;
                                    }
                                    ii++;
                                }
                                break;

                                //case 1 means that all words containing less than the hinted number of occurrences of a letter are no longer usable
                                //also checking for words having the observed letter in the position marked with a '|'
                            case 1:
                                c = map[sideRef[i]].c;
                                while (current->usable == 1 && ii < wordSize) {
                                    if (current->word[ii] == c) {
                                        if (comparison[ii] == '|' &&  current->word[ii] == wordTested[ii]){
                                            current->usable = 0;
                                            usableBlocks--;
                                        }
                                        else count++;
                                    }
                                    ii++;
                                }
                                if (current->usable == 1 && count < map[sideRef[i]].permaCount){
                                    current->usable = 0;
                                    usableBlocks--;
                                }
                                count = 0;
                                break;

                                //case 2 means that all words containing less or more than the hinted number of occurrences of a letter are no longer usable
                                //also checking for words having the observed letter in the position marked with a '|' or '/'
                            case 2:
                                c = map[sideRef[i]].c;
                                while(current->usable == 1 && ii < wordSize) {
                                    if (current->word[ii] == c) {
                                        if (comparison[ii] == '|' && current->word[ii] == wordTested[ii]){
                                            current->usable = 0;
                                            usableBlocks--;
                                        }
                                        else if (comparison[ii] == '/' && current->word[ii] == wordTested[ii]) {
                                            current->usable = 0;
                                            usableBlocks--;
                                        }
                                        else count++;
                                    }
                                    ii++;
                                }
                                if (current->usable == 1 && count != map[sideRef[i]].permaCount) {
                                    current->usable = 0;
                                    usableBlocks--;
                                }
                                count = 0;
                                break;
                        }
                        i++;
                    }
                }
                current = current->right;
            }

            else{
                temp = current->left;

                while(temp->right != NULL && temp->right != current){
                    temp = temp->right;
                }

                if(temp->right == NULL){
                    temp->right = current;
                    current = current->left;
                }
                else{
                    if(current->usable == 1){
                        i = 0;
                        ii = 0;

                        while(current->usable == 1 && ii < wordSize){
                            if(comparison[ii] == '+' && current->word[ii] != wordTested[ii]){
                                current->usable = 0;
                                usableBlocks--;
                            }
                            ii++;
                        }

                        while(current->usable == 1 && i < k){
                            ii= 0;

                            switch(map[sideRef[i]].mode){
                                case 0:
                                    c = map[sideRef[i]].c;
                                    while(current->usable == 1 && ii < wordSize){
                                        if(current->word[ii] == c){
                                            current->usable = 0;
                                            usableBlocks--;
                                        }
                                        ii++;
                                    }
                                    break;
                                case 1:
                                    c = map[sideRef[i]].c;
                                    while (current->usable == 1 && ii < wordSize) {
                                        if (current->word[ii] == c) {
                                            if (comparison[ii] == '|' &&  current->word[ii] == wordTested[ii]){
                                                current->usable = 0;
                                                usableBlocks--;
                                            }
                                            else count++;
                                        }
                                        ii++;
                                    }
                                    if (current->usable == 1 && count < map[sideRef[i]].permaCount){
                                        current->usable = 0;
                                        usableBlocks--;
                                    }
                                    count = 0;
                                    break;
                                case 2:
                                    c = map[sideRef[i]].c;
                                    while(current->usable == 1 && ii < wordSize) {
                                        if (current->word[ii] == c) {
                                            if (comparison[ii] == '|' && current->word[ii] == wordTested[ii]){
                                                current->usable = 0;
                                                usableBlocks--;
                                            }
                                            else if (comparison[ii] == '/' && current->word[ii] == wordTested[ii]) {
                                                current->usable = 0;
                                                usableBlocks--;
                                            }
                                            else count++;
                                        }
                                        ii++;
                                    }
                                    if (current->usable == 1 && count != map[sideRef[i]].permaCount) {
                                        current->usable = 0;
                                        usableBlocks--;
                                    }
                                    count = 0;
                                    break;
                            }
                            i++;
                        }
                    }
                    temp->right = NULL;
                    current = current->right;
                }
            }
        }
    }
    i = 0;
    while(i < wordSize && sideRef[i] != 90){
        map[sideRef[i]].c = '\0';
        map[sideRef[i]].count = 0;
        i++;
    }
}

/**
 * function used to create the hint resulting from a user guess
 * @param curr user's guess
 * @param ref word to guess
 * @param comp hint computed
 * @param side vector containing letters present in the right word (@param ref) guessed correctly from user
 * @param noMap matrix[i][ii] containing 0 or 1 whether a letter (i, following mapFucntion numbering) should be in position ii
 */
void wordCheck(char *curr, char *ref,char *comp, int side[],int noMap[][wordSize]){
    register int  i,k,countRef,toPrint;

    comp[wordSize] = '\0';

    //puts in the output string the "+" symbol (ie where the current guess matches the secret word)
    for(i = 0; i < wordSize; i++){
        if(curr[i] == ref[i]){
            comp[i] =  '+';
        }
        else{
            comp[i]= '?';   //"?" chosen as placeholder (isn't in the game alphabet)
        }
    }

    for(i = 0; i < wordSize; i++){

        //for each word that doesn't match in the guess
        if(comp[i] == '?'){
            countRef = 0;
            //this counts the presence of a letter in reference
            for(k = 0; k < wordSize; k++){
                if(curr[i] == ref[k]) {
                    countRef++;
                }
            }

            toPrint = countRef;
            //this decrements the previous count by the number of letter that match position of the counted letter in the guess
            for(k = 0; k < wordSize; k++){
                if(curr[i] == curr[k] && comp[k] == '+'){
                    toPrint--;
                }
            }

            /*
             * this prints the "|" symbol where there is a letter that is present in the secret word but is not in the guessed position;
             * the presence of a letter and the non-exact matches of such letter are counted because if there are too many instances of
             * such letter then you have to show a "/"
             */
            k = 0;
            while(toPrint > 0 && k < wordSize){
                if (curr[k] == curr[i] && comp[k] =='?'){
                    comp[k] = '|';
                    toPrint--;
                }
                k++;
            }
            //puts the "/" symbol where is needed
            if(toPrint == 0){
                for (k = 0; k < wordSize; k++){
                    if (comp[k] == '?' && curr[k] == curr[i]){
                        comp[k] = '/';
                    }
                }
            }
        }
    }

    //after the above computation the valid words are checked to delete the ones that aren't usable anymore
    if(fastVectorUsage == 0){
        usableCheck(comp, curr, side, noMap);
    }
    else fastLaneCheck(comp,curr,side,noMap);

    puts(comp);
    printf("%d\n", usableBlocks);

}



/**
 * this function is used at start of the game or before the first move to insert new words; it allocates by block as allocating word by word makes more system calls and could then be expensive
 * @param breaker string needed from STDIN to stop the function
 * @return 1 in case of error, 0 otherwise
 */
int insertion(char *breaker){
    struct word *precedent = NULL, *current = treeRoot, *block;
    char new[wordSize+1];
    //counts the words in a block
    int blockCounter = 0;

    //allocate first block
    block = (struct word *) malloc(sizeof(struct word) * maxBlockCount);
    if(block == NULL) return 1;

    //get first word
    if(fgets(new,wordSize+1,stdin) == NULL) return 1;
    getchar();

    //do the same until the breaker string is matched
    while(strcmp(new,breaker) != 0) {

        //allocate new block when needed
        if(blockCounter == maxBlockCount){

            block = (struct word *) malloc(sizeof(struct word) * maxBlockCount);
            if(block == NULL) {
                return 1;
            }
            blockCounter = 0;
        }

        //initialize node
        strcpy(block->word, new);
        block->usable = 1;
        block->right= NULL;
        block->left = NULL;
        blockCount++;
        usableBlocks++;

        //place the node in the right place
        while (current != NULL) {
            precedent = current;
            if (strcmp(new, current->word) < 0) {
                current = current->left;
            } else current = current->right;
        }

        if (precedent == NULL) {
            treeRoot = block;
        }
        else if (strcmp(new, precedent->word) < 0) {
            precedent->left = block;
        }
        else precedent->right = block;

        precedent = NULL;
        current = treeRoot;
        blockCounter++;
        block++;

        if(fgets(new,wordSize+1,stdin) == NULL) return 1;
        getchar();
    }
    while(getchar() != '\n');
    return 0;
}


/**
 * this function is used to insert words during a match; this is needed to not insert invalid (by that point in such match) in the list
 * @param breaker string needed from STDIN to stop the function
 * @param side vector containing letters present in the right word guessed correctly from user
 * @param noMap matrix[i][ii] containing 0 or 1 whether a letter (i, following mapFucntion numbering) should be in position ii
 * @return 1 in case of error, 0 otherwise
 */
int midGameInsertion(char *breaker, int side[], int noMap[][wordSize]) {
    struct word *precedent = NULL, *current = treeRoot, *block = NULL;
    char new[wordSize + 1], c;
    register int ii = 0, blockCounter = 0;
    int k;
    int count = 0;

    //in this case the blocks allocated are smaller as the number of words inserted during a match is usually smaller
    block = (struct word *) malloc(sizeof(struct word) * maxBlockCount / 5);
    if (block == NULL) {
        return 1;
    }

    if (fgets(new, wordSize + 1, stdin) == NULL) return 1;
    getchar();

    //same code from insertion with modified code from usableCheck function
    while (strcmp(new, breaker) != 0) {

        if (blockCounter == (maxBlockCount / 5)) {

            block = (struct word *) malloc(sizeof(struct word) * (maxBlockCount / 5));

            if (block == NULL) {
                return 1;
            }
            blockCounter = 0;
        }
        strcpy(block->word, new);
        block->usable = 1;
        block->right = NULL;
        block->left = NULL;
        blockCount++;

        while (current != NULL) {
            precedent = current;
            if (strcmp(new, current->word) < 0) {
                current = current->left;
            } else current = current->right;
        }

        if (precedent == NULL) {
            treeRoot = block;
        } else if (strcmp(new, precedent->word) < 0) {
            precedent->left = block;
        } else precedent->right = block;

        precedent = NULL;
        current = treeRoot;

        k = 0;
        while (block->usable == 1 && k < wordSize) {
            c = block->word[k];
            if (noMap[mapFunction(c)][k] == 1) {
                block->usable = 0;
            }
            k++;
        }
        k = 0;
        while (block->usable == 1 && side[k] != 90 && k < wordSize) {
            if (map[side[k]].mode == 1) {
                c = reverseMapFunction(side[k]);
                while (ii < wordSize) {
                    if (block->word[ii] == c) {
                        count++;
                    }
                    ii++;
                }
                if (count < map[side[k]].permaCount) {
                    block->usable = 0;
                }
                count = 0;
            } else if (map[side[k]].mode == 2) {
                c = reverseMapFunction(side[k]);
                while (ii < wordSize) {
                    if (block->word[ii] == c) {
                        count++;
                    }
                    ii++;
                }
                if (count != map[side[k]].permaCount) {
                    block->usable = 0;
                }
                count = 0;
            }
            k++;
            ii = 0;
        }
        //if still usable it is inserted
        if (block->usable == 1) {
            usableBlocks++;
            if(fastVectorUsage != 0){
                if (fastVectorUsage < fastVectorLen) {
                    fastVector[fastVectorUsage] = block;
                    fastVectorUsage++;
                } else if (fastVectorUsage == fastVectorLen) fastVectorUsage++;
            }
        }
        if (fastVectorUsage == fastVectorLen + 1) fastVectorUsage = 0;

        block++;
        blockCounter++;
        if (fgets(new, wordSize + 1, stdin) == NULL) return 1;
        while (getchar() != '\n');
    }
    return 0;
}

/**
 * function to reset values before a new game
 */
void resetValue(){
    int i;

    resetSubR(treeRoot);
    for(i = 0; i < 64; i++){
        map[i].c = '\0';
        map[i].count = 0;
        map[i].permaCount = 0;
        map[i].mode = 3;
    }
    fastVectorUsage = 0;
    usableBlocks = blockCount;
    j = 0;
}

/**
 * main function which manages a game and the single turn
 * @param insInit string to start insertion
 * @param insFin string to end insertion
 * @param stmFil string to write usable words
 * @param initPar string to start match
 * @return 0 if game is stopped, 1 if a new match starts
 */
int gameRoutine(char *insInit, char *insFin, char *stmFil,char *initPar){
    register int attempts,first = 1,i,jj;
    int side[wordSize], noMap[64][wordSize];
    char refWord[wordSize+1], buffer[wordSize+1],curr[wordSize+1],comparison[wordSize+1];

    //initialization of "side" vector; this vector is used for
    for(i = 0; i < wordSize; i++){
        side[i] = 90;
    }
    for(i = 0; i < 64; i++){
        for(jj = 0; jj < wordSize; jj++){
            noMap[i][jj] = 0;
        }
    }

    //here is read the word to be guessed; null checks in this function return 2 for error because 1 is already used
    if(fgets(refWord,wordSize+1,stdin) == NULL) return 2;
    while(getchar() != '\n');
    //here is read the number of available attempts
    if(fgets(buffer,wordSize+1,stdin) == NULL) return 2;
    attempts = atoi(buffer);

    //here starts the match
    while(attempts > 0 ){

        //here is where I get the current attempt (or a mid-game command)
        if(fgets(curr,wordSize+1,stdin) == NULL) return 2;
        while(getchar() != '\n');

        //if the number of valid word is low enough the remaining ones are moved to a vector from the bst
        if(usableBlocks <= fastVectorLen && fastVectorUsage == 0){
            fastLaneStock(treeRoot);
        }

        //here is checking whether the line is about the command for adding words
        if(strcmp(curr,insInit) == 0){

            //this distinction is needed because if words are added mid game i need to check they are still valid
            if(first == 1){
                insertion(insFin);
            }
            else{
                //here are used the breaker, side and map for checking validity
                midGameInsertion(insFin, side, noMap);
            }
        }
            //checking for the command to print all valid words
        else if(strcmp(curr,stmFil) == 0){
            //the implemented print only displays valid words
            print(treeRoot);
        }
            //checking the player's guess
        else{
            //case in which the guess is correct
            if(strcmp(curr,refWord) == 0){
                printf("ok\n");

                //picking up new input from user
                if(fgets(curr,wordSize+1,stdin) == NULL) return 2;
                while(getchar() != '\n');

                //check for word insertion command
                if(strcmp(curr,insInit ) == 0){
                    insertion(insFin);
                    if(fgets(curr,wordSize+1,stdin) == NULL) return 2;
                    while(getchar() != '\n');
                }
                //checking for start of game
                if(strcmp(curr,initPar) == 0){
                    resetValue();
                    return 1;
                }
                else return 0;
            }
                //in case of wrong guess, but present in word list
            else if (present(curr) == 1){
                wordCheck(curr,refWord,comparison,side,noMap);
                attempts--;
                first = 0;
            }
                //in case of guess not present in word list
            else {
                printf("not_exists\n");
            }
        }
    }
    //here is the code in case of ko (ie the players hasn't guessed the word)
    printf("ko\n");
    if(fgets(curr,wordSize+1,stdin) == NULL) return 2;
    while(getchar() != '\n');
    if(strcmp(curr,insInit ) == 0){
        insertion(insFin);
        if(fgets(curr,wordSize+1,stdin) == NULL) return 2;
        while(getchar() != '\n');
    }
    if(strcmp(curr,initPar) == 0){
        resetValue();
        return 1;
    }
    else return 0;
}


int main (){
    char c[2],str1[15] = "+nuova_partita",str2[18] = "+inserisci_inizio",str3[16] = "+inserisci_fine", str4[17] = "+stampa_filtrate";
    int i, play = 1;

    //here the len of the word is read from input
    if(fgets(c,2,stdin) == NULL) return 1;
    getchar();
    wordSize = atoi(c);
    maxBlockCount = (4096*64)/(8*3+2);

    char newPartitaCut[wordSize+1],inserisciInizioCut[wordSize+1],inserisciFineCut[wordSize+1],stampaFiltrateCut[wordSize+1];
    for(i = 0;i < wordSize;i++){
        newPartitaCut[i] = str1[i];
        inserisciInizioCut[i] = str2[i];
        inserisciFineCut[i] = str3[i];
        stampaFiltrateCut[i] = str4[i];
    }
    newPartitaCut[wordSize] = '\0';
    inserisciInizioCut[wordSize] = '\0';
    inserisciFineCut[wordSize] = '\0';
    stampaFiltrateCut[wordSize] = '\0';

    //initialization of map
    for(i = 0; i < 64; i++){
        map[i].c = '\0';
        map[i].count = 0;
        map[i].mode = 3;
    }

    insertion(newPartitaCut);

    while(play == 1){
        play = gameRoutine(inserisciInizioCut,inserisciFineCut,stampaFiltrateCut,newPartitaCut);
    }

    return 0;
}