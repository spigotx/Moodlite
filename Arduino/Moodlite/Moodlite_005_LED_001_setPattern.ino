// -----------------------------------------------------------

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void setPattern(int numPattern){
    // add one to the current pattern number, and wrap around at the end
    gCurrentPatternNumber = (numPattern) % ARRAY_SIZE( gPatterns);
}

// -----------------------------------------------------------