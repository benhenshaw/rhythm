# Rendering Text (2018-01-13)
While text in most modern programs is rendered on the fly to allow scaling and differing pixel densities, my program has a fixed internal resolution, so the result of this text rendering is known ahead of time. This reason, and the fact that the implementation is far simpler, lead to my decision to render text the same way I handle animations: an atlas of sub-images.

Firstly, I wanted a way to print anything, including the values of variables. I used the standard library function `vsnprintf` to achieve this. The `v` in its name states that it allows the use of a variadic function argument list to provide its arguments; the `s` states that it will write its result to a string; the `n` means that a hard character limit must be specified. Aside from these aspects, it performs the same function as `printf`: it converts data into strings.

Once I have my string, I must find some way to map the characters to bitmap images in my atlas. I could make a list of characters and their associated `x, y` positions in the bitmap, but this is more time consuming than I would like. The method I used leans on the fact that all characters are of course represented by numbers, and these number have a reliable ordering. At this point I made the concious decision to only support the English language in my initial implementation, and to rely on the ASCII standard for character representation.

An important characteristic of the ASCII standard is that all 'drawable' characters are grouped sequentially. All characters from `!` (value 33) to the character `~` (value 126), are drawable, in that they are not white space or a 'control' character. Knowing this, I could use the character's value to calculate an index into my font bitmap.

In order to keep the rendering process simple, I opted for mono-space fonts, where I will always know the width and height of every character, as they are all equal. Using a font bitmap that packs each glyph horizontally, starting at the space ' ' character (value 32), here is how I calculate the index:

I index the string to get the character I want, then subtract ' ' (32) from it to map it to my index which starts from zero.

```C
int top_left_x = character_width * (text[c] - ' ')
```

This value, and the fact that each glyph is packed horizontally, means that I know where in my texture atlas my glyph is (`top_left_x, 0`), and can render it.

As I mentioned in my previous post, I will also need the `pitch` of my texture atlas to index into it. I could save this separately, but it is derivable from the width of a character. 96 is the difference between '~' (126) and ' ' (32) (the total number of characters in the font bitmap), but it starts from zero so -1.
```C
int pitch_of_font_bitmap = 95 * character_width;
```

This combined with keeping a sum of how far towards the left I move after rendering each character gives me a simple and fast way to render text. I can easily use this render text into a buffer if it is being used frequently.
