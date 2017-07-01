struct Fog {
  vec3 Color;
  float Start;
  float End;
  float Density;
  int Equation; // 0 = linear, 1 = exp, 2 = exp2
};

float CalcFogFactor(Fog params, float FogCoord) 
{
  float Result = 0.0;
  switch(params.Equation) {
    case 0:
      Result = (params.End - FogCoord)/(params.End - params.Start);
      break;
    case 1:
      Result = exp(-params.Density*FogCoord);
      break;
    case 2:
      Result = exp(-params.Density*FogCoord*params.Density*FogCoord);
      break;
    default:
      break;
  }
  Result = 1.0 - clamp(Result, 0.0, 1.0);
  return Result;
}

