#pragma once
struct DataPoint {
    float x;
    float y;
    int category;
    int pointOrTarget = 0; //0 point Clound,1 target, -1 null  0�ǵ㣬1��ȦȦ
    //DataPoint():x(0.0),y(0.0), category(-1), pointOrTarget(-1){}
};
