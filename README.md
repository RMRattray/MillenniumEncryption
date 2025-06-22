# Millennium Encryption

Millennium encryption is an encryption method inspired by the people who believe that, in the infamous Y2K glitch, videocassette players, answering machines,
and flip phones disappeared from all material record, existing exclusively in the minds of those who were alive during the 1990s.  These people call themselves
Millennials, to disassociate from people their age who don't make their birth cohort part of their identity, and to associate with the effortless achievement
of a round number as if it were an accomplishment, as video games (an entertainment medium which wholly disappeared in the same glitch) trained them to do.
Their name inspires the name of this encryption method, and their secret code (perhaps the best disproof of security through obscurity, as it seems more
widely known than Morse or Braille) of repeating digits to indicate letters inspires the encryption method itself.

The method of encryption is not as easily cracked as the ancient, forgotten art of pressing the "2 ABC" key twice for the letter B - it isn't pure substitution,
as it compresses the substitutions in a strange overlapping way inspired by how one might err while typing quickly on such a device.  I implemented it at first
because I felt it was interesting, not consisting wholly of substitution and transposition the way classical ciphers tend to, and imagine it might, in consequence
of this and its operations at the bit level rather than the byte level, be moderately difficult to crack without knowledge of it.  This method does not have the advantage of compressing a message,
nor is it unusually easy to implement, nor (being a classical cipher) is it secure, and though it does have the property that it tends to reduce the number of "high"
bits in the message - perhaps useful in some peculiar analog device wherein the power consumption of high bits is a concern - Huffman coding would presumably do so further,
with a shorter message overall.  In short, this cipher was purely for *fun*.

<img src="images/badlogo.png">