//
//  python_test_shim.c
//  Caption Converter Testing Driver File
//
//  Created by Rob Taylor on 5/2/19.
//

#include "output_utils.h"

/*----------------------------------------------------------------------------*/
/*--                           Global Variables                             --*/
/*----------------------------------------------------------------------------*/

FILE* outputFilePtr[3];

/*----------------------------------------------------------------------------*/
/*--                           Public Functions                             --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 |                       Output Utils Integration Tests
 |                       ==============================
 | TESTED FILES:
 |    output_utils.c
 |
 | PYTHON METHODS:
 |    OpenFiles()
 |    WriteFiles()
 |    CloseFiles()
 -------------------------------------------------------------------------------*/
void OpenFiles( void ) {
    outputFilePtr[0] = fileOutputInit("FileOne.txt");
    outputFilePtr[1] = fileOutputInit("FileTwo.txt");
    outputFilePtr[2] = fileOutputInit("FileThree.txt");
}

void WriteFiles( void ) {
    writeToFile(outputFilePtr[0], "Does all of the printf stuff %s?\n", "work");
    writeToFile(outputFilePtr[0], "Looks like the numbers work %d %d %d %d %d\n", 1, 2, 3, 4, 5);
    writeToFile(outputFilePtr[0], "Looks like even hex numbers work 0x%04X 0x%04X\n", 0xDEAD, 0xBEEF);

    writeToFile(outputFilePtr[1], "[VINCENT] - It's the little differences. A lotta the same s___ we got here, ");
    writeToFile(outputFilePtr[1], "they got there, but there they're a little different.\n");
    writeToFile(outputFilePtr[1], "[JULES] - Example?\n");
    writeToFile(outputFilePtr[1], "[VNCENT] - Alright, when you .... into a movie theatre in Amsterdam, you can ");
    writeToFile(outputFilePtr[1], "buy beer. And I don't mean in a paper cup either. ");
    writeToFile(outputFilePtr[1], "They give you a glass of beer. And in Paris, you can buy beer at MacDonald's. ");
    writeToFile(outputFilePtr[1], "And you know what they call a ");
    writeToFile(outputFilePtr[1], "Quarter Pounder with Cheese in Paris?\n");
    writeToFile(outputFilePtr[1], "[JULES] - They don't call it a Quarter Pounder with Cheese?\n");
    writeToFile(outputFilePtr[1], "[VINCENT] - No, they got the metric system there, they wouldn't know what the ");
    writeToFile(outputFilePtr[1], "f___ a Quarter Pounder is.\n");
    writeToFile(outputFilePtr[1], "[JULES] - What'd they call it?\n");
    writeToFile(outputFilePtr[1], "[VINCENT] - They call it Royale with Cheese.\n");
    writeToFile(outputFilePtr[1], "[JULES] - Royale with Cheese. What'd they call a Big Mac?\n");
    writeToFile(outputFilePtr[1], "[VINCENT] - Big Mac's a Big Mac, but they call it Le Big Mac.\n");

    writeToFile(outputFilePtr[2], "Ezekiel %d:%d. \"The path of the righteous man is beset on all sides by the\n", 25, 17);
    writeToFile(outputFilePtr[2], "inequities of the selfish and the tyranny of evil men. Blessed is he who,\n");
    writeToFile(outputFilePtr[2], "in the name of charity and good will, shepherds the weak through the valley\n");
    writeToFile(outputFilePtr[2], "of the darkness. For he is truly his brother's keeper and the finder of lost\n");
    writeToFile(outputFilePtr[2], "children. And I will strike down upon thee with great vengeance and furious\n");
    writeToFile(outputFilePtr[2], "anger those who attempt to poison and destroy my brothers. And you will know\n");
    writeToFile(outputFilePtr[2], "I am the Lord when I lay my vengeance upon you.\"\n\n");
    writeToFile(outputFilePtr[2], "I been sayin' that s___ for years. And if you ever heard it, it meant your a__.\n");
    writeToFile(outputFilePtr[2], "I never really questioned what it meant. I thought it was just a cold-blooded\n");
    writeToFile(outputFilePtr[2], "thing to say to a m_____f_____ before you popped a cap in his a__. But I saw\n");
    writeToFile(outputFilePtr[2], "some s___ this mornin' made me think twice. Now I'm thinkin': it could mean\n");
    writeToFile(outputFilePtr[2], "you're the evil man. And I'm the righteous man. And Mr. .%d here, he's the\n", 45);
    writeToFile(outputFilePtr[2], "shepherd protecting my righteous a__ in the valley of darkness. Or it could\n");
    writeToFile(outputFilePtr[2], "be you're the righteous man and I'm the shepherd and it's the world that's\n");
    writeToFile(outputFilePtr[2], "evil and selfish. I'd like that. But that s___ ain't the truth. The truth\n");
    writeToFile(outputFilePtr[2], "is you're the weak. And I'm the tyranny of evil men. But I'm tryin, Ringo.\n");
    writeToFile(outputFilePtr[2], "I'm tryin' real hard to be the shepherd.\n");
}

void CloseFiles( void ) {
    closeFile(outputFilePtr[0]);
    closeFile(outputFilePtr[1]);
    closeFile(outputFilePtr[2]);
}
