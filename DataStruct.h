#pragma once
struct DataPoint {
    float x;
    float y;
    int category;
    int pointOrTarget = 0; //0 point Clound,1 target, -1 null  0ÊÇµã£¬1ÊÇÈ¦È¦
    //DataPoint():x(0.0),y(0.0), category(-1), pointOrTarget(-1){}
};
