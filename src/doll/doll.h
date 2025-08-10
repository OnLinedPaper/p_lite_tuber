#ifndef DOLL_H_
#define DOLL_H_

//dolls are going to be FK-esque rigs. i'm going to try to keep them simple:
//each one has a single image (which may be animated) and two "inputs". 
//
//the first input is an (x,y,a) coordinate. ("a" is "angle"). 
//for the "base" doll this will map to a point on the screen, and for all 
//dollparts this will be the "pin" that attaches it to its parent dollpart.
//the second input will be some means to move or manipulate the dollpart.
//
//the pin input can be stiff or flexible in any orientation, i.e. a mouth
//dollpart is probably rigidly fixed in position and angle, whereas an
//arm dollpart might be rigidly position-fixed but flexibly angle-fixed,
//and a flame might be flexibly position-fixed but rigidly angle-fixed.
//
//the other input... i'm not sure how to describe. probably for audio
//stuff right now, but i bet i could find some other uses for it. this input
//can be tweaked based on thresholds to do different things. 
//note that the other input is expecting values between 0.0 and 1.0. 
//
//currently considering each part having an "output" function that lets it
//provide data to others, but i'm not sure what data it could potentially
//provide aside from its own position/rotation (which should be already
//accessible through getters and setters)
//
//although animated sprites are fine, a dollpart may NOT have more than one
//image, for the sake of tidiness. if users want to, say, make a mouth open or
//close based on audio levels, they need to attach two dollparts to the mouth
//area and toggle their transparency levels based on volume thresholds. this
//might make rigging trickier, but it'll keep code simple.

class dollpart {
public:

private:
};

//i'm still trying to decide what to do here. there's a chance i'll just make
//"doll" a handler class that's full of dollparts, which is used to update
//them, pass data along to them, and make sure they draw in the correct order.
//truth be told i feel like there's probably a better way to do this, but i'll
//consider it for the time being...
class doll { 
public:

private:
}

#endif
