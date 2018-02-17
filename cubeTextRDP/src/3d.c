// 3D Defines
#define CULL_NONE 0  // Culling Option: No Polygon Culling
#define CULL_BACK 1  // Culling Option: Back Face Polygon Culling
#define CULL_FRONT 2 // Culling Option: Front Face Polygon Culling

// 3D Functions
typedef struct { float x, y, z; } XYZResult;
typedef struct { float x, y; } XYResult;

// Matrix 3D Data
static float Matrix3D[12] = {
//  X,   Y,   Z,   T
  1.0, 0.0, 0.0, 0.0, // X
  0.0, 1.0, 0.0, 0.0, // Y
  0.0, 0.0, 1.0, 0.0, // Z
};

// Reset Matrix To Identity
void matrix_identity( float matrix[] )
{
  matrix[0] = 1.0;
  matrix[1] = 0.0;
  matrix[2] = 0.0;
  matrix[3] = 0.0;
  matrix[4] = 0.0;
  matrix[5] = 1.0;
  matrix[6] = 0.0;
  matrix[7] = 0.0;
  matrix[8] = 0.0;
  matrix[9] = 0.0;
  matrix[10] = 1.0;
  matrix[11] = 0.0;
}

// Calculate 3D: X,Y,Z
XYZResult calc_3d( float matrix[], float x, float y, float z )
{
  XYZResult res;
  res.x = (matrix[0] * x) + (matrix[1] * y) + (matrix[2] * z) + matrix[3];
  res.y = (matrix[4] * x) + (matrix[5] * y) + (matrix[6] * z) + matrix[7];
  res.z = (matrix[8] * x) + (matrix[9] * y) + (matrix[10] * z) + matrix[11];
  return res;
}

// Calculate 2D: X,Y
XYResult calc_2d( float x, float y, float z )
{
  XYResult res;
  if (z > 0.0) { // Do Not Divide By Zero
    z /= 240.0; // Z = Z / FOV
    res.x = (float)(int)((x / z) + 160.0); // round(X = (X / Z) + (Screen X / 2))
    res.y = (float)(int)(120.0 - (y / z)); // round(Y = (Screen Y / 2) - (Y / Z))
  }
  else {
    res.x = 0.0;
    res.y = 0.0;
  }
  return res;
}

// Test Polygon Winding Direction (IF Triangle Winding > 0.0: Clockwise ELSE: Anti-Clockwise)
int poly_winding( float x1, float y1, float x2, float y2, float x3, float y3 )
{
  int Hdx = x3 - x1; int Hdy = y3 - y1;
  int Mdx = x2 - x1; int Mdy = y2 - y1;
  return Hdx * Mdy - Hdy * Mdx;
}

// Fill Point Array: Vert Array, Color Array, Point Size, Base, Length
void fill_point_array( float vert[], uint8_t col[], uint16_t size, uint32_t base, uint32_t length)
{
  for(uint32_t v = base, c = (base / 3) << 2; v < (base + length); v += 3, c += 4) {
    // Calculate 3D Point
    XYZResult xyz = calc_3d(Matrix3D, vert[v], vert[v + 1], vert[v + 2]);

    // Calculate 2D Point
    XYResult xy = calc_2d(xyz.x, xyz.y, xyz.z);

    rdp_set_blend_color(col[c], col[c + 1], col[c + 2], col[c + 3]); // Set Blend Color: R,G,B,A
    rdp_fill_rectangle( xy.x,xy.y, xy.x + size,xy.y + size ); // Fill Rectangle: XH,YH, XL,YL
    rdp_sync_pipe(); // Stall Pipeline, Until Preceeding Primitives Completely Finish
  }
}

// Fill Z-Buffer Point Array: Vert Array, Color Array, Point Size, Base, Length
void fill_zbuffer_point_array( float vert[], uint8_t col[], uint16_t size, uint32_t base, uint32_t length)
{
  for(uint32_t v = base, c = (base / 3) << 2; v < (base + length); v += 3, c += 4) {
    // Calculate 3D Point
    XYZResult xyz = calc_3d(Matrix3D, vert[v], vert[v + 1], vert[v + 2]);

    // Calculate 2D Point
    XYResult xy = calc_2d(xyz.x, xyz.y, xyz.z);

    rdp_set_blend_color(col[c], col[c + 1], col[c + 2], col[c + 3]); // Set Blend Color: R,G,B,A
    rdp_set_prim_depth(xyz.z,0); // Set Primitive Depth: Primitive Z,Primitive Delta Z
    rdp_fill_rectangle( xy.x,xy.y, xy.x + size,xy.y + size ); // Fill Rectangle: XH,YH, XL,YL
    rdp_sync_pipe(); // Stall Pipeline, Until Preceeding Primitives Completely Finish
  }
}

// Fill Triangle Array: Vert Array, Color Array, Culling, Base, Length
void fill_triangle_array( float vert[], uint8_t col[], uint8_t cull, uint32_t base, uint32_t length)
{
  for(uint32_t v = base, c = (base / 9) << 2; v < (base + length); v += 9, c += 4) {
    // Calculate 3D Points
    XYZResult xyz1 = calc_3d(Matrix3D, vert[v], vert[v + 1], vert[v + 2]);
    XYZResult xyz2 = calc_3d(Matrix3D, vert[v + 3], vert[v + 4], vert[v + 5]);
    XYZResult xyz3 = calc_3d(Matrix3D, vert[v + 6], vert[v + 7], vert[v + 8]);

    // Calculate 2D Points
    XYResult xy1 = calc_2d(xyz1.x, xyz1.y, xyz1.z);
    XYResult xy2 = calc_2d(xyz2.x, xyz2.y, xyz2.z);
    XYResult xy3 = calc_2d(xyz3.x, xyz3.y, xyz3.z);

    // Test Polygon Winding Direction (IF Triangle Winding > 0.0: Clockwise ELSE: Anti-Clockwise)
    int winding = poly_winding(xy1.x,xy1.y, xy2.x,xy2.y, xy3.x,xy3.y);

    if((cull == CULL_NONE) || ((winding <= 0) && (cull == CULL_BACK)) || ((winding > 0) && (cull == CULL_FRONT))) {
      rdp_set_blend_color(col[c], col[c + 1], col[c + 2], col[c + 3]); // Set Blend Color: R,G,B,A
      rdp_draw_fill_triangle( xy1.x,xy1.y, xy2.x,xy2.y, xy3.x,xy3.y ); // Draw Fill Triangle: X1,Y1, X2,Y2, X3,Y3
      rdp_sync_pipe(); // Stall Pipeline, Until Preceeding Primitives Completely Finish
    }
  }
}

// Fill Z-Buffer Triangle Array: Vert Array, Color Array, Culling, Base, Length
void fill_zbuffer_triangle_array( float vert[], uint8_t col[], uint8_t cull, uint32_t base, uint32_t length)
{
  for(uint32_t v = base, c = (base / 9) << 2; v < (base + length); v += 9, c += 4) {
    // Calculate 3D Points
    XYZResult xyz1 = calc_3d(Matrix3D, vert[v], vert[v + 1], vert[v + 2]);
    XYZResult xyz2 = calc_3d(Matrix3D, vert[v + 3], vert[v + 4], vert[v + 5]);
    XYZResult xyz3 = calc_3d(Matrix3D, vert[v + 6], vert[v + 7], vert[v + 8]);

    // Calculate 2D Points
    XYResult xy1 = calc_2d(xyz1.x, xyz1.y, xyz1.z);
    XYResult xy2 = calc_2d(xyz2.x, xyz2.y, xyz2.z);
    XYResult xy3 = calc_2d(xyz3.x, xyz3.y, xyz3.z);

    // Test Polygon Winding Direction (IF Triangle Winding > 0.0: Clockwise ELSE: Anti-Clockwise)
    int winding = poly_winding(xy1.x,xy1.y, xy2.x,xy2.y, xy3.x,xy3.y);

    if((cull == CULL_NONE) || ((winding <= 0) && (cull == CULL_BACK)) || ((winding > 0) && (cull == CULL_FRONT))) {
      rdp_set_blend_color(col[c], col[c + 1], col[c + 2], col[c + 3]); // Set Blend Color: R,G,B,A
      rdp_draw_fill_zbuffer_triangle( xy1.x,xy1.y,xyz1.z, xy2.x,xy2.y,xyz2.z, xy3.x,xy3.y,xyz3.z ); // Draw Fill Triangle: X1,Y1,Z1 X2,Y2,Z2 X3,Y3,Z3
      rdp_sync_pipe(); // Stall Pipeline, Until Preceeding Primitives Completely Finish
    }
  }
}

// Translate: Matrix, X
void translate_x( float matrix[], float x )
{
  matrix[3] = x;
}

// Translate: Matrix, Y
void translate_y( float matrix[], float y )
{
  matrix[7] = y;
}

// Translate: Matrix, Z
void translate_z( float matrix[], float z )
{
  matrix[11] = z;
}

// Translate: Matrix, X, Y, Z
void translate_xyz( float matrix[], float x, float y, float z )
{
  matrix[3] = x;
  matrix[7] = y;
  matrix[11] = z;
}

// Rotate: Matrix, Precalc Table, X
void rotate_x( float matrix[], float precalc[], uint16_t x )
{
  matrix[5] = precalc[(x + 256) & 1023]; // XC
  matrix[6] = -precalc[x]; // -XS
  matrix[9] = -matrix[6]; // XS
  matrix[10] = matrix[5]; // XC
}

// Rotate: Matrix, Precalc Table, Y
void rotate_y( float matrix[], float precalc[], uint16_t y )
{
  matrix[0] = precalc[(y + 256) & 1023]; // YC
  matrix[8] = -precalc[y]; // -YS
  matrix[2] = -matrix[8]; // YS
  matrix[10] = matrix[0]; // YC
}

// Rotate: Matrix, Precalc Table, Z
void rotate_z( float matrix[], float precalc[], uint16_t z )
{
  matrix[0] = precalc[(z + 256) & 1023]; // ZC
  matrix[1] = -precalc[z]; // -ZS
  matrix[4] = -matrix[1]; // ZS
  matrix[5] = matrix[0]; // ZC
}

// Rotate: Matrix, Precalc Table, X, Y
void rotate_xy( float matrix[], float precalc[], uint16_t x, uint16_t y )
{
  matrix[5] = precalc[(x + 256) & 1023]; // XC
  matrix[6] = precalc[x]; // XS
  matrix[0] = precalc[(y + 256) & 1023]; // YC
  matrix[8] = precalc[y]; // YS
  matrix[1] = matrix[6] * matrix[8]; // XS * YS
  matrix[2] = -matrix[5] * matrix[8]; // -XC * YS
  matrix[9] = -matrix[6] * matrix[0]; // -XS * YC
  matrix[10] = matrix[5] * matrix[0]; // XC * YC
}

// Rotate: Matrix, Precalc Table, X, Z
void rotate_xz( float matrix[], float precalc[], uint16_t x, uint16_t z )
{
  matrix[10] = precalc[(x + 256) & 1023]; // XC
  matrix[6] = precalc[x]; // XS
  matrix[0] = precalc[(z + 256) & 1023]; // ZC
  matrix[1] = precalc[z]; // ZS
  matrix[4] = matrix[10] * -matrix[1]; // XC * -ZS
  matrix[5] = matrix[10] * matrix[0]; // XC * ZC
  matrix[8] = matrix[6] * matrix[1]; // XS * ZS
  matrix[9] = matrix[6] * -matrix[0]; // XS * -ZC
}

// Rotate: Matrix, Precalc Table, Y, Z
void rotate_yz( float matrix[], float precalc[], uint16_t y, uint16_t z )
{
  matrix[10] = precalc[(y + 256) & 1023]; // YC
  matrix[8] = precalc[y]; // YS
  matrix[5] = precalc[(z + 256) & 1023]; // ZC
  matrix[1] = precalc[z]; // ZS
  matrix[0] = matrix[10] * matrix[5]; // YC * ZC
  matrix[2] = matrix[8] * -matrix[5]; // YS * -ZC
  matrix[4] = matrix[10] * -matrix[1]; // YC * -ZS
  matrix[6] = matrix[8] * matrix[1]; // YS * ZS
}

// Rotate: Matrix, Precalc Table, X, Y, Z
void rotate_xyz( float matrix[], float precalc[], uint16_t x, uint16_t y, uint16_t z )
{
  matrix[1] = precalc[(x + 256) & 1023]; // XC
  matrix[6] = precalc[x]; // XS
  matrix[10] = precalc[(y + 256) & 1023]; // YC
  matrix[2] = precalc[y]; // YS
  matrix[8] = precalc[(z + 256) & 1023]; // ZC
  matrix[9] = precalc[z]; // ZS
  matrix[5] = -matrix[6] * matrix[10]; // -XS * YC
  matrix[4] = (matrix[5] * matrix[8]) - (matrix[1] * matrix[9]); // (-XS * YC * ZC) - (XC * ZS)
  matrix[5] = (matrix[5] * matrix[9]) + (matrix[1] * matrix[8]); // (-XS * YC * ZS) + (XC * ZC)
  matrix[1] = matrix[1] * matrix[10]; // XC * YC
  matrix[0] = (matrix[1] * matrix[8]) - (matrix[6] * matrix[9]); // (XC * YC * ZC) - (XS * ZS)
  matrix[1] = (matrix[1] * matrix[9]) + (matrix[6] * matrix[8]); // (XC * YC * ZS) + (XS * ZC)
  matrix[6] *= matrix[2]; // XS * YS
  matrix[8] *= matrix[2]; // ZC * YS
  matrix[9] *= matrix[2]; // ZS * YS
  matrix[2] *= -precalc[(x + 256) & 1023]; // -XC * YS
}

static float Sin1024[1024] = { // 1024 Rotations (Sin)
  0.000000000000000000,
  0.006135884649154475,
  0.012271538285719925,
  0.01840672990580482,
  0.024541228522912288,
  0.030674803176636626,
  0.03680722294135883,
  0.04293825693494082,
  0.049067674327418015,
  0.05519524434968994,
  0.06132073630220858,
  0.06744391956366405,
  0.07356456359966743,
  0.07968243797143013,
  0.0857973123444399,
  0.09190895649713272,
  0.0980171403295606,
  0.10412163387205459,
  0.11022220729388306,
  0.11631863091190475,
  0.1224106751992162,
  0.12849811079379317,
  0.13458070850712617,
  0.1406582393328492,
  0.14673047445536175,
  0.15279718525844344,
  0.15885814333386145,
  0.16491312048996992,
  0.17096188876030122,
  0.17700422041214875,
  0.18303988795514095,
  0.1890686641498062,
  0.19509032201612825,
  0.2011046348420919,
  0.20711137619221856,
  0.21311031991609136,
  0.2191012401568698,
  0.22508391135979283,
  0.2310581082806711,
  0.2370236059943672,
  0.24298017990326387,
  0.24892760574572015,
  0.25486565960451457,
  0.2607941179152755,
  0.26671275747489837,
  0.272621355449949,
  0.27851968938505306,
  0.2844075372112719,
  0.29028467725446233,
  0.2961508882436238,
  0.3020059493192281,
  0.30784964004153487,
  0.3136817403988915,
  0.3195020308160157,
  0.3253102921622629,
  0.33110630575987643,
  0.33688985339222005,
  0.3426607173119944,
  0.34841868024943456,
  0.35416352542049034,
  0.3598950365349881,
  0.36561299780477385,
  0.3713171939518375,
  0.37700741021641826,
  0.3826834323650898,
  0.38834504669882625,
  0.3939920400610481,
  0.3996241998456468,
  0.40524131400498986,
  0.4108431710579039,
  0.41642956009763715,
  0.4220002707997997,
  0.4275550934302821,
  0.43309381885315196,
  0.43861623853852766,
  0.4441221445704292,
  0.44961132965460654,
  0.45508358712634384,
  0.46053871095824,
  0.4659764957679662,
  0.47139673682599764,
  0.4767992300633221,
  0.4821837720791227,
  0.487550160148436,
  0.49289819222978404,
  0.4982276669727818,
  0.5035383837257176,
  0.508830142543107,
  0.5141027441932217,
  0.5193559901655896,
  0.524589682678469,
  0.5298036246862946,
  0.5349976198870972,
  0.5401714727298929,
  0.5453249884220465,
  0.5504579729366048,
  0.5555702330196022,
  0.560661576197336,
  0.5657318107836131,
  0.5707807458869673,
  0.5758081914178453,
  0.5808139580957645,
  0.5857978574564389,
  0.5907597018588742,
  0.5956993044924334,
  0.600616479383869,
  0.6055110414043255,
  0.6103828062763095,
  0.6152315905806268,
  0.6200572117632891,
  0.6248594881423863,
  0.629638238914927,
  0.6343932841636455,
  0.6391244448637757,
  0.6438315428897914,
  0.6485144010221124,
  0.6531728429537768,
  0.6578066932970786,
  0.6624157775901718,
  0.6669999223036375,
  0.6715589548470183,
  0.6760927035753159,
  0.680600997795453,
  0.6850836677727004,
  0.6895405447370668,
  0.6939714608896539,
  0.6983762494089729,
  0.7027547444572253,
  0.7071067811865475,
  0.7114321957452164,
  0.7157308252838186,
  0.7200025079613817,
  0.7242470829514669,
  0.7284643904482252,
  0.7326542716724128,
  0.7368165688773698,
  0.7409511253549591,
  0.745057785441466,
  0.7491363945234593,
  0.7531867990436125,
  0.7572088465064846,
  0.7612023854842618,
  0.765167265622459,
  0.7691033376455796,
  0.7730104533627369,
  0.7768884656732324,
  0.7807372285720944,
  0.7845565971555752,
  0.7883464276266062,
  0.7921065773002123,
  0.7958369046088836,
  0.799537269107905,
  0.8032075314806448,
  0.8068475535437992,
  0.8104571982525948,
  0.8140363297059483,
  0.8175848131515837,
  0.8211025149911046,
  0.8245893027850253,
  0.8280450452577558,
  0.8314696123025452,
  0.83486287498638,
  0.838224705554838,
  0.8415549774368983,
  0.844853565249707,
  0.8481203448032971,
  0.8513551931052652,
  0.8545579883654005,
  0.8577286100002721,
  0.8608669386377673,
  0.8639728561215867,
  0.8670462455156926,
  0.8700869911087113,
  0.8730949784182901,
  0.8760700941954066,
  0.8790122264286334,
  0.8819212643483549,
  0.8847970984309378,
  0.8876396204028539,
  0.8904487232447579,
  0.8932243011955153,
  0.8959662497561851,
  0.8986744656939538,
  0.901348847046022,
  0.9039892931234433,
  0.9065957045149153,
  0.9091679830905224,
  0.9117060320054299,
  0.9142097557035307,
  0.9166790599210427,
  0.9191138516900578,
  0.9215140393420419,
  0.9238795325112867,
  0.9262102421383114,
  0.9285060804732156,
  0.9307669610789837,
  0.9329927988347388,
  0.9351835099389475,
  0.937339011912575,
  0.9394592236021899,
  0.9415440651830208,
  0.9435934581619604,
  0.9456073253805213,
  0.9475855910177411,
  0.9495281805930367,
  0.9514350209690083,
  0.9533060403541938,
  0.9551411683057707,
  0.9569403357322089,
  0.9587034748958716,
  0.9604305194155658,
  0.9621214042690416,
  0.9637760657954398,
  0.9653944416976894,
  0.9669764710448521,
  0.9685220942744173,
  0.970031253194544,
  0.9715038909862518,
  0.9729399522055601,
  0.9743393827855759,
  0.9757021300385286,
  0.9770281426577544,
  0.9783173707196277,
  0.9795697656854405,
  0.9807852804032304,
  0.9819638691095552,
  0.9831054874312163,
  0.984210092386929,
  0.9852776423889412,
  0.9863080972445987,
  0.9873014181578584,
  0.9882575677307495,
  0.989176509964781,
  0.9900582102622971,
  0.99090263542778,
  0.9917097536690995,
  0.99247953459871,
  0.9932119492347945,
  0.9939069700023561,
  0.9945645707342554,
  0.9951847266721968,
  0.9957674144676598,
  0.996312612182778,
  0.9968202992911657,
  0.9972904566786902,
  0.9977230666441916,
  0.9981181129001492,
  0.9984755805732948,
  0.9987954562051724,
  0.9990777277526454,
  0.9993223845883495,
  0.9995294175010931,
  0.9996988186962042,
  0.9998305817958234,
  0.9999247018391445,
  0.9999811752826011,
  1.0000000000000000,
  0.9999811752826011,
  0.9999247018391445,
  0.9998305817958234,
  0.9996988186962042,
  0.9995294175010931,
  0.9993223845883495,
  0.9990777277526454,
  0.9987954562051724,
  0.9984755805732948,
  0.9981181129001492,
  0.9977230666441916,
  0.9972904566786902,
  0.9968202992911658,
  0.996312612182778,
  0.9957674144676598,
  0.9951847266721969,
  0.9945645707342554,
  0.9939069700023561,
  0.9932119492347945,
  0.99247953459871,
  0.9917097536690995,
  0.99090263542778,
  0.9900582102622971,
  0.989176509964781,
  0.9882575677307495,
  0.9873014181578584,
  0.9863080972445987,
  0.9852776423889412,
  0.984210092386929,
  0.9831054874312163,
  0.9819638691095552,
  0.9807852804032304,
  0.9795697656854405,
  0.9783173707196277,
  0.9770281426577544,
  0.9757021300385286,
  0.9743393827855759,
  0.9729399522055602,
  0.9715038909862518,
  0.970031253194544,
  0.9685220942744173,
  0.9669764710448521,
  0.9653944416976894,
  0.9637760657954398,
  0.9621214042690416,
  0.9604305194155659,
  0.9587034748958716,
  0.9569403357322089,
  0.9551411683057707,
  0.9533060403541939,
  0.9514350209690083,
  0.9495281805930367,
  0.9475855910177412,
  0.9456073253805214,
  0.9435934581619604,
  0.9415440651830208,
  0.9394592236021899,
  0.937339011912575,
  0.9351835099389476,
  0.9329927988347388,
  0.9307669610789837,
  0.9285060804732156,
  0.9262102421383114,
  0.9238795325112867,
  0.921514039342042,
  0.9191138516900578,
  0.9166790599210427,
  0.9142097557035307,
  0.9117060320054299,
  0.9091679830905225,
  0.9065957045149153,
  0.9039892931234434,
  0.901348847046022,
  0.8986744656939539,
  0.8959662497561852,
  0.8932243011955152,
  0.890448723244758,
  0.8876396204028539,
  0.8847970984309379,
  0.881921264348355,
  0.8790122264286335,
  0.8760700941954066,
  0.8730949784182902,
  0.8700869911087115,
  0.8670462455156928,
  0.8639728561215868,
  0.8608669386377672,
  0.8577286100002721,
  0.8545579883654005,
  0.8513551931052652,
  0.8481203448032972,
  0.8448535652497072,
  0.8415549774368984,
  0.8382247055548382,
  0.8348628749863801,
  0.8314696123025453,
  0.8280450452577558,
  0.8245893027850252,
  0.8211025149911048,
  0.8175848131515837,
  0.8140363297059485,
  0.8104571982525948,
  0.8068475535437994,
  0.8032075314806449,
  0.7995372691079052,
  0.7958369046088836,
  0.7921065773002123,
  0.7883464276266063,
  0.7845565971555751,
  0.7807372285720946,
  0.7768884656732324,
  0.7730104533627371,
  0.7691033376455796,
  0.7651672656224591,
  0.7612023854842619,
  0.7572088465064847,
  0.7531867990436125,
  0.7491363945234593,
  0.7450577854414661,
  0.740951125354959,
  0.73681656887737,
  0.7326542716724128,
  0.7284643904482253,
  0.7242470829514669,
  0.7200025079613818,
  0.7157308252838187,
  0.7114321957452167,
  0.7071067811865476,
  0.7027547444572252,
  0.6983762494089729,
  0.6939714608896539,
  0.689540544737067,
  0.6850836677727004,
  0.6806009977954532,
  0.6760927035753159,
  0.6715589548470186,
  0.6669999223036376,
  0.662415777590172,
  0.6578066932970787,
  0.6531728429537766,
  0.6485144010221126,
  0.6438315428897914,
  0.6391244448637758,
  0.6343932841636455,
  0.6296382389149272,
  0.6248594881423863,
  0.6200572117632894,
  0.6152315905806269,
  0.6103828062763097,
  0.6055110414043255,
  0.6006164793838689,
  0.5956993044924335,
  0.5907597018588742,
  0.585797857456439,
  0.5808139580957645,
  0.5758081914178454,
  0.5707807458869673,
  0.5657318107836135,
  0.5606615761973361,
  0.5555702330196022,
  0.5504579729366049,
  0.5453249884220464,
  0.540171472729893,
  0.5349976198870972,
  0.5298036246862948,
  0.524589682678469,
  0.5193559901655898,
  0.5141027441932218,
  0.5088301425431073,
  0.5035383837257177,
  0.49822766697278176,
  0.49289819222978415,
  0.4875501601484359,
  0.4821837720791229,
  0.4767992300633221,
  0.47139673682599786,
  0.4659764957679662,
  0.4605387109582402,
  0.4550835871263439,
  0.4496113296546069,
  0.4441221445704293,
  0.43861623853852755,
  0.43309381885315207,
  0.42755509343028203,
  0.42200027079979985,
  0.41642956009763715,
  0.41084317105790413,
  0.4052413140049899,
  0.39962419984564707,
  0.39399204006104815,
  0.3883450466988266,
  0.3826834323650899,
  0.37700741021641815,
  0.3713171939518377,
  0.3656129978047738,
  0.35989503653498833,
  0.3541635254204904,
  0.3484186802494348,
  0.34266071731199443,
  0.33688985339222033,
  0.3311063057598765,
  0.32531029216226326,
  0.3195020308160158,
  0.3136817403988914,
  0.30784964004153503,
  0.30200594931922803,
  0.296150888243624,
  0.2902846772544624,
  0.2844075372112721,
  0.27851968938505317,
  0.27262135544994925,
  0.2667127574748985,
  0.26079411791527585,
  0.2548656596045147,
  0.2489276057457201,
  0.24298017990326407,
  0.23702360599436717,
  0.23105810828067133,
  0.22508391135979283,
  0.21910124015687005,
  0.21311031991609142,
  0.20711137619221884,
  0.201104634842092,
  0.1950903220161286,
  0.18906866414980636,
  0.1830398879551409,
  0.17700422041214894,
  0.17096188876030122,
  0.16491312048997014,
  0.15885814333386147,
  0.15279718525844369,
  0.1467304744553618,
  0.14065823933284954,
  0.13458070850712628,
  0.12849811079379309,
  0.12241067519921635,
  0.11631863091190471,
  0.11022220729388324,
  0.10412163387205457,
  0.09801714032956083,
  0.09190895649713275,
  0.08579731234444016,
  0.0796824379714302,
  0.07356456359966773,
  0.06744391956366418,
  0.06132073630220849,
  0.055195244349690094,
  0.049067674327417966,
  0.04293825693494102,
  0.03680722294135883,
  0.030674803176636865,
  0.024541228522912326,
  0.0184067299058051,
  0.012271538285720007,
  0.006135884649154799,
  0.000000000000000000,
  -0.006135884649154554,
  -0.012271538285719762,
  -0.01840672990580486,
  -0.02454122852291208,
  -0.03067480317663662,
  -0.03680722294135858,
  -0.04293825693494078,
  -0.049067674327417724,
  -0.05519524434968985,
  -0.061320736302208245,
  -0.06744391956366393,
  -0.0735645635996675,
  -0.07968243797142995,
  -0.08579731234443992,
  -0.09190895649713252,
  -0.09801714032956059,
  -0.10412163387205432,
  -0.110222207293883,
  -0.11631863091190447,
  -0.1224106751992161,
  -0.12849811079379284,
  -0.13458070850712606,
  -0.1406582393328493,
  -0.14673047445536158,
  -0.15279718525844344,
  -0.15885814333386122,
  -0.1649131204899699,
  -0.17096188876030097,
  -0.1770042204121487,
  -0.18303988795514065,
  -0.1890686641498061,
  -0.19509032201612836,
  -0.20110463484209176,
  -0.2071113761922186,
  -0.2131103199160912,
  -0.2191012401568698,
  -0.2250839113597926,
  -0.23105810828067108,
  -0.23702360599436695,
  -0.24298017990326382,
  -0.24892760574571987,
  -0.25486565960451446,
  -0.2607941179152756,
  -0.26671275747489825,
  -0.27262135544994903,
  -0.2785196893850529,
  -0.2844075372112718,
  -0.2902846772544621,
  -0.2961508882436238,
  -0.3020059493192278,
  -0.3078496400415348,
  -0.3136817403988912,
  -0.3195020308160156,
  -0.325310292162263,
  -0.33110630575987626,
  -0.33688985339222005,
  -0.3426607173119942,
  -0.34841868024943456,
  -0.3541635254204901,
  -0.3598950365349881,
  -0.3656129978047736,
  -0.37131719395183743,
  -0.3770074102164179,
  -0.38268343236508967,
  -0.38834504669882636,
  -0.39399204006104793,
  -0.39962419984564684,
  -0.4052413140049897,
  -0.4108431710579039,
  -0.41642956009763693,
  -0.4220002707997996,
  -0.4275550934302818,
  -0.43309381885315185,
  -0.4386162385385273,
  -0.4441221445704291,
  -0.44961132965460665,
  -0.45508358712634367,
  -0.46053871095824006,
  -0.46597649576796596,
  -0.47139673682599764,
  -0.4767992300633219,
  -0.48218377207912266,
  -0.48755016014843566,
  -0.4928981922297839,
  -0.49822766697278154,
  -0.5035383837257175,
  -0.5088301425431071,
  -0.5141027441932216,
  -0.5193559901655895,
  -0.5245896826784687,
  -0.5298036246862946,
  -0.5349976198870969,
  -0.5401714727298929,
  -0.5453249884220461,
  -0.5504579729366047,
  -0.555570233019602,
  -0.5606615761973359,
  -0.5657318107836132,
  -0.5707807458869671,
  -0.5758081914178453,
  -0.5808139580957643,
  -0.5857978574564389,
  -0.5907597018588739,
  -0.5956993044924332,
  -0.6006164793838686,
  -0.6055110414043254,
  -0.6103828062763095,
  -0.6152315905806267,
  -0.6200572117632892,
  -0.6248594881423862,
  -0.629638238914927,
  -0.6343932841636453,
  -0.6391244448637757,
  -0.6438315428897913,
  -0.6485144010221123,
  -0.6531728429537765,
  -0.6578066932970785,
  -0.6624157775901718,
  -0.6669999223036374,
  -0.6715589548470184,
  -0.6760927035753158,
  -0.680600997795453,
  -0.6850836677727001,
  -0.6895405447370668,
  -0.6939714608896538,
  -0.6983762494089728,
  -0.7027547444572251,
  -0.7071067811865475,
  -0.7114321957452164,
  -0.7157308252838185,
  -0.7200025079613817,
  -0.7242470829514668,
  -0.7284643904482252,
  -0.7326542716724127,
  -0.7368165688773698,
  -0.7409511253549589,
  -0.7450577854414658,
  -0.749136394523459,
  -0.7531867990436124,
  -0.7572088465064842,
  -0.761202385484262,
  -0.765167265622459,
  -0.7691033376455795,
  -0.7730104533627367,
  -0.7768884656732326,
  -0.7807372285720944,
  -0.784556597155575,
  -0.7883464276266059,
  -0.7921065773002124,
  -0.7958369046088835,
  -0.7995372691079048,
  -0.803207531480645,
  -0.8068475535437992,
  -0.8104571982525947,
  -0.8140363297059481,
  -0.8175848131515838,
  -0.8211025149911046,
  -0.8245893027850251,
  -0.8280450452577555,
  -0.8314696123025452,
  -0.8348628749863799,
  -0.8382247055548379,
  -0.8415549774368986,
  -0.8448535652497071,
  -0.8481203448032971,
  -0.8513551931052649,
  -0.8545579883654005,
  -0.857728610000272,
  -0.8608669386377671,
  -0.8639728561215865,
  -0.8670462455156926,
  -0.8700869911087113,
  -0.8730949784182899,
  -0.8760700941954067,
  -0.8790122264286334,
  -0.8819212643483549,
  -0.8847970984309376,
  -0.887639620402854,
  -0.8904487232447579,
  -0.8932243011955152,
  -0.8959662497561849,
  -0.8986744656939538,
  -0.9013488470460219,
  -0.9039892931234431,
  -0.9065957045149154,
  -0.9091679830905224,
  -0.9117060320054298,
  -0.9142097557035305,
  -0.9166790599210427,
  -0.9191138516900577,
  -0.9215140393420418,
  -0.9238795325112865,
  -0.9262102421383114,
  -0.9285060804732155,
  -0.9307669610789836,
  -0.932992798834739,
  -0.9351835099389476,
  -0.9373390119125748,
  -0.9394592236021897,
  -0.9415440651830208,
  -0.9435934581619603,
  -0.9456073253805212,
  -0.9475855910177412,
  -0.9495281805930367,
  -0.9514350209690083,
  -0.9533060403541938,
  -0.9551411683057708,
  -0.9569403357322088,
  -0.9587034748958715,
  -0.9604305194155657,
  -0.9621214042690416,
  -0.9637760657954398,
  -0.9653944416976893,
  -0.9669764710448522,
  -0.9685220942744173,
  -0.970031253194544,
  -0.9715038909862517,
  -0.9729399522055602,
  -0.9743393827855759,
  -0.9757021300385285,
  -0.9770281426577543,
  -0.9783173707196277,
  -0.9795697656854405,
  -0.9807852804032303,
  -0.9819638691095554,
  -0.9831054874312163,
  -0.984210092386929,
  -0.9852776423889411,
  -0.9863080972445987,
  -0.9873014181578583,
  -0.9882575677307495,
  -0.9891765099647809,
  -0.9900582102622971,
  -0.99090263542778,
  -0.9917097536690995,
  -0.9924795345987101,
  -0.9932119492347945,
  -0.9939069700023561,
  -0.9945645707342554,
  -0.9951847266721969,
  -0.9957674144676598,
  -0.996312612182778,
  -0.9968202992911657,
  -0.9972904566786902,
  -0.9977230666441916,
  -0.9981181129001492,
  -0.9984755805732948,
  -0.9987954562051724,
  -0.9990777277526454,
  -0.9993223845883494,
  -0.9995294175010931,
  -0.9996988186962042,
  -0.9998305817958234,
  -0.9999247018391445,
  -0.9999811752826011,
  -1.0000000000000000,
  -0.9999811752826011,
  -0.9999247018391445,
  -0.9998305817958234,
  -0.9996988186962042,
  -0.9995294175010931,
  -0.9993223845883495,
  -0.9990777277526454,
  -0.9987954562051724,
  -0.9984755805732948,
  -0.9981181129001492,
  -0.9977230666441916,
  -0.9972904566786902,
  -0.9968202992911657,
  -0.996312612182778,
  -0.9957674144676598,
  -0.9951847266721969,
  -0.9945645707342554,
  -0.9939069700023561,
  -0.9932119492347946,
  -0.9924795345987101,
  -0.9917097536690995,
  -0.99090263542778,
  -0.9900582102622971,
  -0.9891765099647809,
  -0.9882575677307495,
  -0.9873014181578584,
  -0.9863080972445988,
  -0.9852776423889412,
  -0.9842100923869291,
  -0.9831054874312164,
  -0.9819638691095554,
  -0.9807852804032304,
  -0.9795697656854405,
  -0.9783173707196278,
  -0.9770281426577543,
  -0.9757021300385286,
  -0.974339382785576,
  -0.9729399522055603,
  -0.9715038909862518,
  -0.970031253194544,
  -0.9685220942744174,
  -0.9669764710448523,
  -0.9653944416976894,
  -0.96377606579544,
  -0.9621214042690417,
  -0.9604305194155658,
  -0.9587034748958716,
  -0.9569403357322089,
  -0.9551411683057709,
  -0.9533060403541938,
  -0.9514350209690084,
  -0.9495281805930368,
  -0.9475855910177413,
  -0.9456073253805213,
  -0.9435934581619604,
  -0.9415440651830209,
  -0.9394592236021898,
  -0.937339011912575,
  -0.9351835099389477,
  -0.9329927988347391,
  -0.9307669610789837,
  -0.9285060804732156,
  -0.9262102421383115,
  -0.9238795325112866,
  -0.9215140393420419,
  -0.9191138516900579,
  -0.9166790599210428,
  -0.9142097557035306,
  -0.9117060320054299,
  -0.9091679830905225,
  -0.9065957045149156,
  -0.9039892931234433,
  -0.901348847046022,
  -0.898674465693954,
  -0.895966249756185,
  -0.8932243011955153,
  -0.890448723244758,
  -0.8876396204028542,
  -0.8847970984309377,
  -0.881921264348355,
  -0.8790122264286336,
  -0.8760700941954069,
  -0.8730949784182901,
  -0.8700869911087115,
  -0.8670462455156929,
  -0.8639728561215866,
  -0.8608669386377673,
  -0.8577286100002722,
  -0.8545579883654008,
  -0.8513551931052651,
  -0.8481203448032973,
  -0.8448535652497072,
  -0.8415549774368988,
  -0.838224705554838,
  -0.8348628749863801,
  -0.8314696123025455,
  -0.8280450452577557,
  -0.8245893027850253,
  -0.8211025149911049,
  -0.8175848131515839,
  -0.8140363297059483,
  -0.8104571982525949,
  -0.8068475535437994,
  -0.8032075314806453,
  -0.799537269107905,
  -0.7958369046088837,
  -0.7921065773002126,
  -0.7883464276266061,
  -0.7845565971555752,
  -0.7807372285720947,
  -0.7768884656732328,
  -0.7730104533627369,
  -0.7691033376455797,
  -0.7651672656224592,
  -0.7612023854842622,
  -0.7572088465064846,
  -0.7531867990436126,
  -0.7491363945234597,
  -0.7450577854414658,
  -0.7409511253549591,
  -0.73681656887737,
  -0.7326542716724131,
  -0.7284643904482251,
  -0.724247082951467,
  -0.7200025079613819,
  -0.715730825283819,
  -0.7114321957452164,
  -0.7071067811865477,
  -0.7027547444572256,
  -0.6983762494089727,
  -0.693971460889654,
  -0.6895405447370672,
  -0.6850836677727008,
  -0.680600997795453,
  -0.676092703575316,
  -0.6715589548470187,
  -0.666999922303638,
  -0.6624157775901718,
  -0.6578066932970789,
  -0.6531728429537771,
  -0.6485144010221123,
  -0.6438315428897915,
  -0.639124444863776,
  -0.6343932841636459,
  -0.629638238914927,
  -0.6248594881423865,
  -0.6200572117632894,
  -0.6152315905806274,
  -0.6103828062763095,
  -0.6055110414043257,
  -0.6006164793838693,
  -0.5956993044924332,
  -0.5907597018588743,
  -0.5857978574564391,
  -0.580813958095765,
  -0.5758081914178452,
  -0.5707807458869674,
  -0.5657318107836136,
  -0.5606615761973366,
  -0.5555702330196022,
  -0.550457972936605,
  -0.5453249884220468,
  -0.5401714727298927,
  -0.5349976198870973,
  -0.5298036246862949,
  -0.5245896826784694,
  -0.5193559901655895,
  -0.5141027441932219,
  -0.5088301425431074,
  -0.5035383837257181,
  -0.49822766697278187,
  -0.49289819222978426,
  -0.4875501601484364,
  -0.4821837720791226,
  -0.4767992300633222,
  -0.4713967368259979,
  -0.4659764957679667,
  -0.46053871095823995,
  -0.455083587126344,
  -0.449611329654607,
  -0.4441221445704298,
  -0.43861623853852766,
  -0.43309381885315223,
  -0.42755509343028253,
  -0.42200027079979957,
  -0.41642956009763726,
  -0.41084317105790424,
  -0.4052413140049904,
  -0.3996241998456468,
  -0.39399204006104827,
  -0.3883450466988267,
  -0.3826834323650904,
  -0.37700741021641826,
  -0.3713171939518378,
  -0.36561299780477435,
  -0.359895036534988,
  -0.3541635254204905,
  -0.3484186802494349,
  -0.34266071731199493,
  -0.33688985339222,
  -0.3311063057598766,
  -0.32531029216226337,
  -0.31950203081601547,
  -0.3136817403988915,
  -0.30784964004153514,
  -0.3020059493192286,
  -0.29615088824362373,
  -0.2902846772544625,
  -0.2844075372112722,
  -0.27851968938505367,
  -0.2726213554499489,
  -0.2667127574748986,
  -0.26079411791527596,
  -0.2548656596045144,
  -0.2489276057457202,
  -0.24298017990326418,
  -0.23702360599436773,
  -0.231058108280671,
  -0.22508391135979297,
  -0.21910124015687016,
  -0.21311031991609197,
  -0.20711137619221853,
  -0.20110463484209212,
  -0.19509032201612872,
  -0.18906866414980603,
  -0.183039887955141,
  -0.17700422041214905,
  -0.17096188876030177,
  -0.1649131204899698,
  -0.15885814333386158,
  -0.1527971852584438,
  -0.1467304744553624,
  -0.1406582393328492,
  -0.13458070850712642,
  -0.12849811079379364,
  -0.12241067519921603,
  -0.11631863091190484,
  -0.11022220729388336,
  -0.10412163387205513,
  -0.0980171403295605,
  -0.09190895649713288,
  -0.08579731234444028,
  -0.07968243797143075,
  -0.07356456359966741,
  -0.06744391956366429,
  -0.06132073630220906,
  -0.055195244349689775,
  -0.04906767432741809,
  -0.04293825693494114,
  -0.036807222941359394,
  -0.030674803176636543,
  -0.024541228522912448,
  -0.018406729905805226,
  -0.012271538285720572,
  -0.006135884649154477,
};