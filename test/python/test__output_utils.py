#!/usr/bin/env python
# -*- coding: latin-1 -*- 

import ctypes
import difflib
import os

CAPTION_INSPECTOR_LIBRARY = './libci-test.1.0.0.dylib'


FileOneText = '''Does all of the printf stuff work?
Looks like the numbers work 1 2 3 4 5
Looks like even hex numbers work 0xDEAD 0xBEEF
'''.splitlines(keepends=True)

FileTwoText = '''[VINCENT] - It's the little differences. A lotta the same s___ we got here, they got there, but there they're a little different.
[JULES] - Example?
[VNCENT] - Alright, when you .... into a movie theatre in Amsterdam, you can buy beer. And I don't mean in a paper cup either. They give you a glass of beer. And in Paris, you can buy beer at MacDonald's. And you know what they call a Quarter Pounder with Cheese in Paris?
[JULES] - They don't call it a Quarter Pounder with Cheese?
[VINCENT] - No, they got the metric system there, they wouldn't know what the f___ a Quarter Pounder is.
[JULES] - What'd they call it?
[VINCENT] - They call it Royale with Cheese.
[JULES] - Royale with Cheese. What'd they call a Big Mac?
[VINCENT] - Big Mac's a Big Mac, but they call it Le Big Mac.
'''.splitlines(keepends=True)

FileThreeText = '''Ezekiel 25:17. "The path of the righteous man is beset on all sides by the
inequities of the selfish and the tyranny of evil men. Blessed is he who,
in the name of charity and good will, shepherds the weak through the valley
of the darkness. For he is truly his brother's keeper and the finder of lost
children. And I will strike down upon thee with great vengeance and furious
anger those who attempt to poison and destroy my brothers. And you will know
I am the Lord when I lay my vengeance upon you."

I been sayin' that s___ for years. And if you ever heard it, it meant your a__.
I never really questioned what it meant. I thought it was just a cold-blooded
thing to say to a m_____f_____ before you popped a cap in his a__. But I saw
some s___ this mornin' made me think twice. Now I'm thinkin': it could mean
you're the evil man. And I'm the righteous man. And Mr. .45 here, he's the
shepherd protecting my righteous a__ in the valley of darkness. Or it could
be you're the righteous man and I'm the shepherd and it's the world that's
evil and selfish. I'd like that. But that s___ ain't the truth. The truth
is you're the weak. And I'm the tyranny of evil men. But I'm tryin, Ringo.
I'm tryin' real hard to be the shepherd.
'''.splitlines(keepends=True)


def compare_text(first, second):
    retval = True
    for loop in range(len(first)):
        if first[loop] != second[loop]:
            retval = False
            print("Different: ")
            d = difflib.Differ()
            diff = d.compare(first[loop], second[loop])
            print("".join(diff))
    return retval


class TestClass(object):
    def test__Output_Utils_Write_Files(self):
        clib = ctypes.CDLL(CAPTION_INSPECTOR_LIBRARY)
        clib.OpenFiles()
        clib.WriteFiles()
        clib.CloseFiles()
        file = open("FileOne.txt", "r") 
        file_one_actual = file.read().splitlines(keepends=True)
        identical = compare_text(file_one_actual, FileOneText)
        assert identical is True
        file.close()
        file = open("FileTwo.txt", "r")
        file_one_actual = file.read().splitlines(keepends=True)
        identical = compare_text(file_one_actual, FileTwoText)
        assert identical is True
        file.close()
        file = open("FileThree.txt", "r")
        file_one_actual = file.read().splitlines(keepends=True)
        identical = compare_text(file_one_actual, FileThreeText)
        assert identical is True
        file.close()
        os.remove("FileOne.txt")
        os.remove("FileTwo.txt")
        os.remove("FileThree.txt")


if __name__ == "__main__":
    pass
