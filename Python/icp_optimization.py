import matplotlib.pyplot as plt
import numpy as np
import math
from sklearn.neighbors import NearestNeighbors

def parse_raw_data(raw_data):
    datasets = []
    current_dataset = []

    for line in raw_data.strip().split('\n'):
        if line.strip() == "Rotate Right" or line.strip() == "Rotate Left":
            if current_dataset:
                datasets.append(current_dataset)
                current_dataset = []
        else:
            points = [int(x) for x in line.split(',')]
            if len(points) == 64:
                xy_pairs = [[points[i], points[i + 32]] for i in range(32)]
                current_dataset.append(xy_pairs)

    if current_dataset:
        datasets.append(current_dataset)

    return datasets

raw_data = """
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
74,90,0,0,0,0,0,0,0,0,0,34,37,40,49,59,-47,-48,-46,0,0,0,0,0,-46,-46,-46,-45,-45,-44,-44,-45,73,74,9,9,9,9,9,9,9,9,9,103,88,77,77,77,64,76,88,9,9,9,9,9,12,17,21,25,30,35,40,47
64,81,0,0,0,0,0,0,0,0,0,33,36,37,42,51,-47,-48,-46,0,0,0,0,0,-46,-45,-46,-46,-45,-45,-44,-45,76,79,20,20,20,20,20,20,20,20,20,110,98,84,79,79,75,87,98,20,20,20,20,20,23,28,32,36,41,46,51,58
54,67,0,0,0,0,0,0,0,0,0,34,37,38,38,41,-47,-46,-47,0,0,0,0,0,-46,-45,-46,-46,-45,-45,-45,-46,77,79,31,31,31,31,31,31,31,31,31,125,110,96,85,80,86,96,112,31,31,31,31,31,34,39,43,47,52,57,62,69
42,0,0,0,0,0,0,0,12,0,0,36,36,37,38,38,-47,-48,-48,0,0,0,-31,-12,-46,-45,-46,-45,-45,-45,-44,-45,79,43,43,43,43,43,43,43,260,43,43,144,121,107,96,88,98,110,125,43,43,43,252,252,46,51,55,59,64,69,74,81
40,41,0,0,0,0,0,0,11,30,0,34,37,37,38,38,-47,-47,-47,0,0,-48,-29,-11,-46,-45,-46,-46,-45,-45,-45,-45,89,84,55,55,55,55,55,55,259,261,55,152,135,119,109,101,110,121,135,55,55,250,252,252,58,63,67,71,76,81,86,93
41,41,42,42,0,0,0,0,10,29,48,35,37,38,38,38,-46,-47,-47,0,-66,-45,-28,-10,-46,-45,-46,-46,-45,-44,-44,-45,100,95,89,85,66,66,66,66,259,263,261,161,146,131,119,112,119,132,146,66,250,248,251,250,69,74,78,82,87,92,97,104
0,41,42,42,42,43,43,0,10,27,46,30,35,37,36,38,-48,-48,-48,0,-61,-43,-26,-10,-46,-45,-46,-46,-45,-45,-45,-45,77,106,100,96,92,88,85,77,259,259,263,162,152,140,129,122,133,144,159,77,248,249,250,251,80,85,89,93,98,103,108,115
41,40,41,42,42,43,43,43,9,26,43,62,33,36,38,38,-48,-48,-54,0,-57,-41,-24,-9,-46,-46,-46,-46,-45,-45,-45,-46,123,117,112,108,104,100,97,92,258,263,265,263,163,152,141,134,145,156,180,89,248,251,252,252,92,97,101,105,110,115,120,127
39,40,41,42,42,43,43,44,9,24,40,58,32,36,36,37,-49,-51,-50,0,-54,-37,-22,-8,-46,-46,-47,-46,-46,-46,-45,-46,134,130,125,121,117,113,110,106,260,262,265,265,172,163,154,146,159,174,187,102,251,253,253,253,105,110,114,118,123,128,133,141
39,39,40,42,41,43,42,44,9,22,37,53,72,33,35,36,-51,-49,-52,-62,-49,-35,-21,-8,-47,-46,-47,-46,-47,-46,-47,-48,146,141,137,133,129,125,121,118,258,259,263,265,269,170,164,157,173,182,202,247,251,253,251,252,118,122,126,130,135,140,147,154
38,39,39,41,41,42,42,43,8,21,35,50,66,84,0,35,-53,-54,-70,-57,-45,-32,-19,-8,-47,-47,-47,-48,-48,-48,-48,-48,158,153,148,145,141,137,133,129,259,260,265,265,268,267,126,167,187,201,243,248,252,253,253,252,130,134,139,143,148,153,159,166
37,38,39,40,40,41,41,41,7,19,32,45,60,77,0,0,-53,-57,-63,-53,-40,-29,-18,-7,-48,-48,-48,-49,-49,-49,-49,-50,167,163,159,155,152,148,144,141,257,261,264,263,265,266,137,137,199,215,244,250,251,251,251,253,141,145,150,155,159,166,171,179
0,0,38,38,38,40,40,41,7,17,29,41,54,70,91,0,-57,-60,-58,-47,-37,-26,-16,-6,-49,-49,-50,-50,-50,-50,-51,-52,149,149,170,166,163,160,156,153,258,261,264,265,266,268,272,149,215,233,248,251,252,253,255,253,153,157,162,167,171,178,185,194
0,0,0,0,37,39,39,39,7,15,26,37,49,63,79,101,-58,-60,-53,-42,-33,-24,-14,-6,-50,-50,-51,-51,-52,-50,-51,-52,160,160,160,160,174,170,167,164,258,261,263,263,265,265,269,274,228,244,250,252,252,252,253,254,164,169,173,178,183,189,196,205
108,0,0,0,0,0,0,44,6,13,23,33,44,55,70,91,-60,-57,-47,-37,-29,-21,-13,-5,-51,-53,-52,-52,-53,-52,-52,-54,266,171,171,171,171,171,171,175,257,259,262,262,265,265,268,275,240,249,251,252,252,251,252,253,175,179,185,190,195,201,209,217
94,0,0,0,0,0,0,0,5,12,20,29,38,48,62,78,-56,-48,-40,-32,-26,-18,-12,-5,-52,-53,-53,-52,-54,-53,-53,-55,264,181,181,181,181,181,181,181,257,258,260,262,262,264,266,270,245,248,250,251,251,251,252,252,185,189,195,200,205,211,219,228
79,0,0,0,0,0,0,0,4,10,18,24,32,40,52,65,-50,-41,-34,-28,-21,-16,-10,-5,-53,-53,-53,-53,-81,-83,-53,-55,261,192,192,192,192,192,192,192,256,257,259,260,262,262,264,266,250,250,250,252,252,252,253,254,196,201,206,212,230,241,230,239
68,0,0,0,0,0,0,0,4,9,15,20,26,33,42,53,-40,-33,-27,-22,-18,-13,-8,-4,-53,-53,-54,-54,-84,-56,-54,-48,262,204,204,204,204,204,204,204,255,257,258,259,261,261,263,265,251,251,252,252,253,254,253,254,208,213,219,224,243,236,242,244
54,0,0,0,0,0,0,0,3,7,11,15,20,26,33,41,-30,-24,-21,-17,-14,-10,-7,-4,-54,-55,-57,-85,-83,-50,-44,-37,261,215,215,215,215,215,215,215,256,256,258,258,259,260,262,264,252,251,253,253,253,253,254,254,219,225,229,246,253,244,246,245
41,52,0,0,0,0,0,0,3,6,8,11,14,17,23,0,-21,-17,-15,-12,-10,-8,-5,-3,-55,-55,-85,-83,-45,-37,-30,-26,261,262,226,226,226,226,226,226,256,256,257,257,259,259,259,226,253,253,253,253,254,254,255,255,230,236,248,256,247,246,247,246
Rotate Right
47,42,0,0,0,0,0,0,0,0,0,112,93,81,68,59,39,47,60,70,0,0,0,214,3,6,8,10,12,16,19,24,169,165,226,226,226,226,226,226,226,226,226,184,181,177,175,174,257,258,259,257,226,226,226,236,252,253,253,253,253,254,254,255
50,46,4,4,4,4,4,4,4,4,4,121,99,84,73,63,43,52,65,80,4,4,4,219,7,10,12,14,16,20,23,4,169,163,225,225,225,225,225,225,225,225,225,182,179,177,174,173,256,257,260,259,225,225,225,236,251,252,252,252,252,253,253,259
52,48,13,13,13,13,13,13,13,13,126,107,93,80,70,52,61,72,92,13,213,218,220,16,18,21,24,26,29,34,39,168,166,161,224,224,224,224,224,224,224,224,183,179,176,174,173,255,257,257,260,224,273,253,234,252,252,252,252,253,253,253,254
69,62,56,50,24,24,24,24,225,231,24,131,116,102,90,81,64,72,86,24,24,219,221,223,27,30,32,36,38,41,45,50,168,167,166,163,223,223,223,223,211,191,223,183,178,176,174,172,256,256,258,223,223,270,251,233,252,253,253,254,253,254,253,254
78,72,66,59,35,35,35,35,227,230,35,35,126,111,100,91,76,87,101,35,215,217,220,223,38,41,44,47,49,52,57,62,169,168,166,165,222,222,222,222,211,192,222,222,179,177,174,173,256,259,260,222,285,266,249,231,253,253,253,253,253,254,253,254
89,83,76,69,66,60,46,46,226,232,235,46,136,122,110,101,88,98,111,46,217,218,220,221,49,52,55,58,60,65,68,74,168,167,166,165,164,163,221,221,210,192,173,221,178,176,174,172,256,256,258,221,281,263,246,230,252,253,253,253,253,253,253,254
99,92,86,80,76,70,65,60,226,231,236,242,57,129,118,109,103,114,130,213,215,218,220,222,61,63,66,69,72,77,81,87,169,168,167,165,164,162,162,160,210,193,174,152,220,176,174,173,257,259,262,292,275,259,243,228,252,253,253,253,254,254,254,255
106,100,95,89,86,81,76,71,223,229,233,237,67,134,127,118,116,125,145,210,216,217,218,221,70,73,77,81,83,87,93,99,170,169,168,166,165,164,163,162,209,194,176,157,219,178,175,174,259,259,264,285,272,256,240,226,253,254,254,254,254,255,255,256
116,110,105,99,95,90,85,80,223,227,232,232,77,77,135,127,127,141,184,213,213,217,218,221,81,83,87,91,93,98,103,109,170,169,168,166,165,165,164,162,209,195,178,161,218,218,175,174,259,263,280,280,265,252,238,225,253,253,254,254,254,255,255,255
124,118,112,107,103,98,93,89,222,224,231,232,239,86,86,135,137,149,210,211,215,216,217,247,89,93,96,100,103,108,113,120,170,168,167,166,165,164,163,162,208,195,180,163,144,217,217,173,259,261,290,274,262,249,235,225,253,254,254,254,255,255,255,257
96,127,122,117,113,108,104,99,221,226,229,231,234,240,96,96,151,164,206,212,215,215,217,219,100,103,107,111,115,119,125,133,216,169,167,166,165,164,164,163,208,196,181,167,150,128,216,216,262,264,280,269,257,244,233,222,253,254,255,255,256,256,257,259
107,107,132,127,123,119,115,110,222,223,229,233,235,237,107,107,168,201,209,213,214,216,217,218,111,114,118,122,127,133,138,147,215,215,169,167,166,166,164,163,207,196,183,168,153,136,215,215,266,283,274,263,253,241,231,221,255,256,257,257,257,258,259,261
120,120,120,120,135,132,127,123,220,223,227,231,231,237,120,120,186,207,212,214,216,235,218,219,124,127,132,136,141,147,153,162,214,214,214,214,167,166,165,164,206,197,186,172,160,143,214,214,270,276,267,257,247,242,228,219,257,257,258,258,258,260,261,263
134,134,134,134,134,134,134,137,220,223,227,229,232,234,241,134,205,211,214,215,217,217,218,220,138,142,147,152,156,163,170,181,213,213,213,213,213,213,213,163,207,198,189,178,166,152,133,213,273,268,259,249,242,233,225,218,259,259,260,261,261,261,264,267
147,147,147,147,147,147,147,147,241,243,226,227,230,234,240,147,211,212,239,234,235,216,219,219,151,156,160,166,171,179,188,198,212,212,212,212,212,212,212,212,205,196,190,182,172,160,143,212,267,257,265,251,243,229,222,216,260,261,263,263,264,266,269,270
159,159,159,159,159,159,159,159,217,220,223,225,227,229,235,159,212,212,214,215,235,236,238,241,163,168,174,179,184,192,200,202,211,211,211,211,211,211,211,211,205,200,193,186,179,169,154,211,256,248,242,235,238,230,223,215,262,263,264,265,266,267,268,261
171,171,171,171,171,171,171,171,217,219,221,222,225,226,231,171,230,231,215,215,217,217,218,219,175,180,186,192,198,202,204,205,210,210,210,210,210,210,210,210,205,201,196,190,185,177,166,210,260,252,233,229,225,221,217,213,263,266,266,267,267,263,257,249
184,184,184,184,184,184,184,184,217,218,220,220,222,223,226,228,216,216,217,218,218,219,219,220,188,193,199,204,205,206,206,206,209,209,209,209,209,209,209,209,205,201,198,194,191,185,179,170,234,230,227,223,220,218,215,212,266,266,265,262,254,246,241,236
Rotate Right
128,184,184,184,184,184,184,184,184,184,152,146,143,139,136,135,224,227,184,184,184,184,184,195,213,213,214,214,216,216,216,184,162,209,209,209,209,209,209,209,209,209,87,106,122,133,143,153,159,146,209,209,209,209,209,-6,205,202,200,197,194,191,185,209
128,183,183,183,183,183,183,183,183,183,152,146,142,138,136,134,193,194,183,183,202,197,192,186,212,213,213,214,214,215,215,183,160,206,206,206,206,206,206,206,206,206,86,105,118,130,141,149,189,186,206,206,152,151,150,147,202,199,197,193,191,186,182,206
129,126,182,182,182,182,182,182,170,182,182,145,141,137,135,133,221,226,182,182,182,182,211,193,213,213,214,215,216,216,217,182,152,157,196,196,196,196,196,196,-19,196,196,95,110,121,132,140,147,132,196,196,196,196,-10,-13,192,189,186,183,180,175,171,196
130,129,181,181,181,181,181,181,169,181,181,145,142,139,136,134,224,230,181,181,247,229,209,191,215,216,216,216,217,218,219,181,142,148,184,184,184,184,184,184,-21,184,184,89,103,113,122,131,131,113,184,184,-4,-10,-16,-18,180,177,173,169,167,161,157,184
130,129,128,180,180,180,180,180,168,149,153,146,142,139,136,135,228,234,180,260,243,224,207,189,216,217,218,218,219,220,221,180,131,137,142,172,172,172,172,172,-24,-28,68,80,92,102,112,120,114,95,172,-1,-6,-9,-13,-18,168,164,161,156,153,148,142,172
132,131,129,127,127,126,124,179,168,150,179,147,143,140,138,136,231,179,179,254,238,220,204,188,219,220,221,221,222,223,225,179,121,126,132,136,140,145,150,159,-24,-31,159,73,84,94,102,109,96,159,159,-3,-7,-12,-16,-23,154,151,147,143,138,132,125,159
134,132,131,129,128,126,126,124,167,151,178,149,145,141,139,138,237,178,178,248,233,217,201,186,222,223,245,246,226,226,229,233,110,115,120,124,128,133,137,142,-26,-31,146,67,76,85,93,100,76,146,146,-5,-10,-14,-18,-21,141,137,128,120,123,116,109,97
135,134,132,131,129,128,127,125,167,152,134,177,146,142,140,139,239,177,257,242,228,213,198,185,223,224,247,227,227,228,230,177,99,105,110,113,117,121,126,130,-20,-29,-36,134,68,76,83,90,60,134,-1,-7,-11,-13,-18,-22,129,125,115,115,110,103,95,134
136,134,133,132,130,129,128,126,167,152,136,118,176,143,141,139,239,176,251,236,223,210,196,183,247,249,227,229,230,231,234,176,90,93,99,102,106,110,114,118,-23,-26,-34,-36,122,67,74,80,47,122,-5,-8,-14,-15,-18,-21,116,109,107,101,96,88,79,122
137,136,135,133,132,130,129,128,166,153,138,120,175,175,143,142,175,255,243,230,218,205,194,182,248,251,231,231,233,235,236,240,81,85,89,92,96,100,103,108,-23,-29,-32,-41,111,111,67,72,111,0,-5,-9,-11,-14,-18,-22,104,98,96,89,83,74,66,53
139,138,136,135,134,132,131,129,166,154,140,122,174,174,174,143,254,247,237,223,214,202,191,180,252,254,235,235,236,238,240,247,71,75,79,82,84,88,92,95,-23,-26,-33,-41,99,99,99,63,6,-3,-9,-10,-14,-15,-19,-21,93,85,83,76,69,60,50,33
173,139,138,136,136,134,133,131,165,154,142,127,110,173,173,173,251,239,228,218,208,198,188,179,255,256,235,236,238,240,243,249,87,65,68,72,73,77,80,84,-23,-27,-32,-38,-44,87,87,87,-3,-6,-8,-12,-15,-17,-21,-22,80,73,70,63,56,46,35,19
172,172,172,172,137,136,135,134,164,155,144,131,117,172,172,172,242,231,221,211,204,195,185,177,256,238,238,240,241,245,248,245,74,74,74,74,61,64,68,71,-23,-26,-32,-35,-42,74,74,74,-7,-8,-12,-14,-16,-19,-22,-23,67,62,55,48,41,30,17,9
171,171,171,171,171,171,171,122,164,156,147,135,123,106,171,171,231,222,214,205,198,195,186,176,238,240,242,244,247,248,244,235,62,62,62,62,62,62,62,58,-23,-26,-31,-35,-38,-45,62,62,-8,-10,-13,-14,-17,-38,-39,-24,56,50,42,34,25,15,8,5
170,170,170,170,170,170,170,170,164,157,149,140,129,170,170,170,221,213,206,199,193,191,183,174,240,243,246,245,246,243,235,225,50,50,50,50,50,50,50,50,-22,-25,-28,-31,-36,50,50,50,-11,-12,-14,-16,-18,-39,-41,-23,44,37,29,21,13,6,2,1
169,169,169,169,169,169,169,169,163,158,151,143,135,124,108,169,211,204,200,193,189,188,180,173,264,244,247,246,241,231,223,215,39,39,39,39,39,39,39,39,-22,-24,-26,-29,-34,-37,-42,39,-12,-13,-16,-17,-18,-38,-41,-23,32,26,18,10,5,1,0,-1
168,168,168,168,168,168,168,168,163,159,152,146,139,130,115,100,203,198,193,188,184,185,178,171,244,245,244,236,226,217,210,205,29,29,29,29,29,29,29,29,-20,-23,-24,-27,-31,-33,-42,-47,-14,-14,-16,-18,-19,-40,-41,-23,23,16,8,3,1,0,-2,-2
167,167,167,167,167,167,167,167,162,159,155,150,144,137,128,167,194,190,186,182,180,177,173,170,244,238,228,219,210,204,199,195,19,19,19,19,19,19,19,19,-19,-21,-24,-24,-28,-29,-33,19,-16,-16,-18,-18,-19,-21,-22,-22,13,7,3,-1,-1,-3,-4,-5
166,166,166,166,166,166,166,166,162,159,157,153,150,145,166,166,186,182,181,178,176,173,171,168,235,211,233,199,196,192,189,186,9,9,9,9,9,9,9,9,-19,-20,-21,-22,-24,-26,9,9,-18,-18,-19,-20,-21,-21,-22,-23,3,0,-8,-4,-5,-6,-7,-7
Rotate Right
123,128,166,166,166,166,166,166,166,46,65,78,88,97,105,114,108,166,166,-13,-19,-24,-27,-36,162,159,156,153,150,145,166,166,59,61,9,9,9,9,9,9,9,27,34,41,45,49,52,54,-39,9,9,-74,-57,-37,-19,-1,-21,-21,-22,-23,-25,-25,9,9
118,123,161,161,161,161,161,161,161,49,58,74,83,93,101,109,105,161,161,-16,-27,-30,-36,-42,157,154,151,147,145,140,161,125,59,61,9,9,9,9,9,9,9,26,35,40,45,49,52,54,-37,9,9,-73,-57,-37,-19,-1,-22,-23,-23,-24,-25,-27,9,-31
108,114,150,150,150,150,150,150,150,34,53,66,76,85,92,100,90,150,150,-22,-30,-32,-42,150,146,143,139,135,132,126,120,109,59,60,9,9,9,9,9,9,9,26,34,39,42,47,50,52,-41,9,9,-71,-54,-35,-18,9,-25,-27,-28,-28,-29,-31,-33,-37
99,105,109,139,139,139,139,139,-51,139,46,60,70,76,85,91,74,139,-19,-27,-32,-38,-43,139,134,131,127,123,118,112,107,97,56,58,60,9,9,9,9,9,20,9,33,37,40,45,47,49,-46,9,-84,-68,-51,-34,-17,9,-28,-30,-32,-32,-33,-34,-35,-39
89,94,100,104,108,127,127,127,-51,127,127,53,61,68,75,81,57,127,-23,-29,-33,-35,-44,-45,122,118,114,110,105,99,92,81,54,56,57,60,61,9,9,9,19,9,9,36,39,43,45,47,-50,9,-80,-63,-47,-31,-15,0,-31,-33,-35,-35,-36,-38,-39,-42
80,85,90,94,98,103,107,112,-50,-54,116,47,54,62,68,74,43,116,-24,-30,-31,-38,-43,-49,112,107,102,97,93,85,77,65,51,53,55,57,59,61,62,63,18,34,9,34,37,40,43,45,-53,9,-74,-59,-43,-28,-13,0,-36,-37,-38,-39,-40,-42,-42,-48
72,77,82,85,89,93,97,102,-48,-54,-65,45,50,57,62,66,106,-19,-26,-32,-31,-41,-43,-46,101,96,92,85,81,72,64,47,49,51,53,54,56,57,59,61,18,33,52,31,34,38,40,43,9,-82,-68,-54,-39,-27,-13,1,-39,-40,-42,-43,-45,-47,-48,-58
65,69,72,76,79,83,87,91,-50,-57,-66,95,43,49,54,58,95,-24,-29,-32,-36,-40,-47,-46,95,95,95,95,95,95,95,95,46,48,49,51,53,55,56,58,17,32,50,9,32,35,38,39,9,-77,-64,-49,-37,-24,-12,1,9,9,9,9,9,9,9,9
55,59,62,66,68,72,76,80,-51,-56,-58,83,83,42,46,50,-21,-25,-32,-35,-37,-39,-44,-49,83,83,83,83,83,83,83,83,43,44,47,48,49,51,53,55,17,30,45,9,9,32,35,36,-81,-69,-58,-45,-33,-21,-9,1,9,9,9,9,9,9,9,9
46,48,51,55,56,60,63,66,-50,-55,-62,70,70,70,37,40,-26,-30,-33,-36,-40,-44,-47,-52,70,70,70,70,70,70,70,70,40,41,43,45,46,47,49,50,17,28,42,9,9,9,31,33,-75,-64,-51,-39,-30,-18,-8,2,9,9,9,9,9,9,9,9
36,39,41,44,46,49,52,55,-50,-54,-61,-66,58,58,58,58,-28,-30,-35,-38,-41,-45,-48,-51,58,58,58,58,58,58,58,58,36,38,40,40,42,43,45,46,16,26,39,54,9,9,9,9,-65,-54,-44,-35,-25,-16,-7,2,9,9,9,9,9,9,9,9
47,30,33,35,36,39,41,43,-49,-53,-61,-64,-71,47,47,47,-28,-34,-36,-38,-41,-44,-49,-50,47,47,47,47,47,47,47,47,9,34,35,36,38,39,41,42,15,25,36,50,64,9,9,9,-56,-48,-38,-29,-22,-13,-4,3,9,9,9,9,9,9,9,9
-104,-85,37,37,37,29,31,34,37,37,37,37,37,37,37,37,-31,-33,-36,-39,-42,-44,-46,-49,37,37,37,37,37,37,37,37,168,175,9,9,9,36,37,37,9,9,9,9,9,9,9,9,-48,-40,-33,-25,-18,-10,-3,3,9,9,9,9,9,9,9,9
-112,-95,-78,-60,26,26,26,22,26,26,26,26,26,26,26,26,-35,-36,-39,-41,-43,-46,-48,-51,21,16,12,5,1,-7,-15,-32,165,174,184,193,9,9,9,53,9,9,9,9,9,9,9,9,-42,-35,-28,-21,-15,-9,-2,4,-39,-40,-42,-43,-45,-47,-48,-58
-121,-104,-87,-69,-52,15,15,15,15,15,15,15,15,15,15,15,-37,-37,-41,-42,-44,-47,-49,-51,8,1,-7,-17,-23,-28,-30,-31,163,172,181,190,199,9,9,9,9,9,9,9,9,9,9,9,-34,-28,-22,-16,-11,-5,0,5,-70,-73,-76,-79,-72,-64,-52,-43
-133,-114,-97,-79,-63,-44,3,3,3,3,3,3,3,3,3,3,-39,-42,-43,-45,-47,-48,-50,-51,-3,-10,-19,-26,-29,-30,-33,-34,162,169,177,187,194,204,9,9,9,9,9,9,9,9,9,9,-26,-21,-16,-12,-8,-3,0,5,-76,-78,-79,-71,-60,-47,-41,-34
-7,-122,-103,-87,-71,-54,-36,-7,-7,-7,-7,-7,-7,-7,-7,-7,-42,-43,-45,-45,-46,-48,-49,-51,-13,-21,-27,-7,-30,-33,-35,-36,9,166,171,183,191,202,213,9,9,9,9,9,9,9,9,9,-18,-14,-11,-7,-5,-1,2,5,-79,-77,-71,9,-42,-35,-30,-25
Rotate Right
"""

datasets = parse_raw_data(raw_data)

numpy_datasets = []
for dataset in datasets:
    numpy_dataset = np.array(dataset)
    numpy_datasets.append(numpy_dataset)

#for i, dataset in enumerate(numpy_datasets):
#    print(f"Dataset {i+1}:\n", len(dataset))
    
#    for j, element in enumerate(dataset):
#        print(f"element {j+1}:\n", len(element))

# 플로팅
plt.figure()  # 새로운 figure 객체 생성

x_coords = numpy_datasets[0][:, :, 0].flatten()
y_coords = numpy_datasets[0][:, :, 1].flatten()
plt.scatter(x_coords, y_coords, color='blue', marker='.')

x_coords = numpy_datasets[1][:, :, 0].flatten()
y_coords = numpy_datasets[1][:, :, 1].flatten()
plt.scatter(x_coords, y_coords, color='blue', marker='.')

x_coords = numpy_datasets[2][:, :, 0].flatten()
y_coords = numpy_datasets[2][:, :, 1].flatten()
plt.scatter(x_coords, y_coords, color='blue', marker='.')

x_coords = numpy_datasets[3][:, :, 0].flatten()
y_coords = numpy_datasets[3][:, :, 1].flatten()
plt.scatter(x_coords, y_coords, color='blue', marker='.')

plt.xlabel('X coordinates')
plt.ylabel('Y coordinates')
plt.title('Plot of unoptimized map')
plt.legend()
plt.axis('equal')
plt.grid(True)
plt.show()

def euclidean_distance(point1, point2):
    """
    Euclidean distance between two points.
    :param point1: the first point as a tuple (a_1, a_2, ..., a_n)
    :param point2: the second point as a tuple (b_1, b_2, ..., b_n)
    :return: the Euclidean distance
    """
    a = np.array(point1)
    b = np.array(point2)

    return np.linalg.norm(a - b, ord=2)


def point_based_matching(point_pairs):
    """
    This function is based on the paper "Robot Pose Estimation in Unknown Environments by Matching 2D Range Scans"
    by F. Lu and E. Milios.

    :param point_pairs: the matched point pairs [((x1, y1), (x1', y1')), ..., ((xi, yi), (xi', yi')), ...]
    :return: the rotation angle and the 2D translation (x, y) to be applied for matching the given pairs of points
    """

    x_mean = 0
    y_mean = 0
    xp_mean = 0
    yp_mean = 0
    n = len(point_pairs)

    if n == 0:
        return None, None, None

    for pair in point_pairs:

        (x, y), (xp, yp) = pair

        x_mean += x
        y_mean += y
        xp_mean += xp
        yp_mean += yp

    x_mean /= n
    y_mean /= n
    xp_mean /= n
    yp_mean /= n

    s_x_xp = 0
    s_y_yp = 0
    s_x_yp = 0
    s_y_xp = 0
    for pair in point_pairs:

        (x, y), (xp, yp) = pair

        s_x_xp += (x - x_mean)*(xp - xp_mean)
        s_y_yp += (y - y_mean)*(yp - yp_mean)
        s_x_yp += (x - x_mean)*(yp - yp_mean)
        s_y_xp += (y - y_mean)*(xp - xp_mean)

    rot_angle = math.atan2(s_x_yp - s_y_xp, s_x_xp + s_y_yp)
    translation_x = xp_mean - (x_mean*math.cos(rot_angle) - y_mean*math.sin(rot_angle))
    translation_y = yp_mean - (x_mean*math.sin(rot_angle) + y_mean*math.cos(rot_angle))

    return rot_angle, translation_x, translation_y


def icp(reference_points, points, max_iterations=100, distance_threshold=6, convergence_translation_threshold=1e-3,
        convergence_rotation_threshold=1e-4, point_pairs_threshold=5, verbose=False):
    """
    An implementation of the Iterative Closest Point algorithm that matches a set of M 2D points to another set
    of N 2D (reference) points.

    :param reference_points: the reference point set as a numpy array (N x 2)
    :param points: the point that should be aligned to the reference_points set as a numpy array (M x 2)
    :param max_iterations: the maximum number of iteration to be executed
    :param distance_threshold: the distance threshold between two points in order to be considered as a pair
    :param convergence_translation_threshold: the threshold for the translation parameters (x and y) for the
                                              transformation to be considered converged
    :param convergence_rotation_threshold: the threshold for the rotation angle (in rad) for the transformation
                                               to be considered converged
    :param point_pairs_threshold: the minimum number of point pairs the should exist
    :param verbose: whether to print informative messages about the process (default: False)
    :return: the transformation history as a list of numpy arrays containing the rotation (R) and translation (T)
             transformation in each iteration in the format [R | T] and the aligned points as a numpy array M x 2
    """

    transformation_history = []

    nbrs = NearestNeighbors(n_neighbors=1, algorithm='kd_tree').fit(reference_points)

    for iter_num in range(max_iterations):
        if verbose:
            print('------ iteration', iter_num, '------')

        closest_point_pairs = []  # list of point correspondences for closest point rule

        distances, indices = nbrs.kneighbors(points)
        for nn_index in range(len(distances)):
            if distances[nn_index][0] < distance_threshold:
                closest_point_pairs.append((points[nn_index], reference_points[indices[nn_index][0]]))

        # if only few point pairs, stop process
        if verbose:
            print('number of pairs found:', len(closest_point_pairs))
        if len(closest_point_pairs) < point_pairs_threshold:
            if verbose:
                print('No better solution can be found (very few point pairs)!')
            break

        # compute translation and rotation using point correspondences
        closest_rot_angle, closest_translation_x, closest_translation_y = point_based_matching(closest_point_pairs)
        if closest_rot_angle is not None:
            if verbose:
                print('Rotation:', math.degrees(closest_rot_angle), 'degrees')
                print('Translation:', closest_translation_x, closest_translation_y)
        if closest_rot_angle is None or closest_translation_x is None or closest_translation_y is None:
            if verbose:
                print('No better solution can be found!')
            break

        # transform 'points' (using the calculated rotation and translation)
        c, s = math.cos(closest_rot_angle), math.sin(closest_rot_angle)
        rot = np.array([[c, -s],
                        [s, c]])
        aligned_points = np.dot(points, rot.T)
        aligned_points[:, 0] += closest_translation_x
        aligned_points[:, 1] += closest_translation_y

        # update 'points' for the next iteration
        points = aligned_points

        # update transformation history
        transformation_history.append(np.hstack((rot, np.array([[closest_translation_x], [closest_translation_y]]))))

        # check convergence
        if (abs(closest_rot_angle) < convergence_rotation_threshold) \
                and (abs(closest_translation_x) < convergence_translation_threshold) \
                and (abs(closest_translation_y) < convergence_translation_threshold):
            if verbose:
                print('Converged!')
            break

    return transformation_history, points    

def compute_optimal_transformation(source_points, target_points):
    """
    Compute the optimal transformation (rotation and translation) between
    source and target points using Singular Value Decomposition (SVD).
    """
    centroid_source = np.mean(source_points, axis=0)
    centroid_target = np.mean(target_points, axis=0)
    
    centered_source = source_points - centroid_source
    centered_target = target_points - centroid_target
    
    H = np.dot(centered_source.T, centered_target)
    U, S, Vt = np.linalg.svd(H)
    
    R = np.dot(Vt.T, U.T)
    t = centroid_target - np.dot(R, centroid_source)
    
    return R, t

def apply_transformation(points, R, t):
    """
    Apply the transformation (rotation R and translation t) to the points.
    """
    transformed_points = np.dot(points, R.T) + t
    return transformed_points

def icp_sequential(source_points, target_points, max_iterations=10, distance_threshold=10, convergence_translation_threshold=1e-3,
        convergence_rotation_threshold=1e-4, verbose=False):
    """
    An implementation of the Iterative Closest Point algorithm that matches a set of M 2D points to another set
    of N 2D (reference) points.

    :param source_points: the source point set as a numpy array (N x 2)
    :param target_points: the target point set as a numpy array (M x 2)
    :param max_iterations: the maximum number of iteration to be executed
    :param distance_threshold: the distance threshold between two points in order to be considered as a pair
    :param convergence_translation_threshold: the threshold for the translation parameters (x and y) for the
                                              transformation to be considered converged
    :param convergence_rotation_threshold: the threshold for the rotation angle (in rad) for the transformation
                                               to be considered converged
    :param verbose: whether to print informative messages about the process (default: False)
    :return: the transformation history as a list of numpy arrays containing the rotation (R) and translation (T)
             transformation in each iteration in the format [R | T] and the aligned points as a numpy array M x 2
    """

    transformation_history = []

    for iter_num in range(max_iterations):
        if verbose:
            print('------ iteration', iter_num, '------')

        # compute translation and rotation using point correspondences
        R, t = compute_optimal_transformation(source_points, target_points)
        if verbose:
            print('Rotation:', R)
            print('Translation:', t)

        # transform 'source_points' (using the calculated rotation and translation)
        source_points = apply_transformation(source_points, R, t)

        # update transformation history
        transformation_history.append(np.hstack((R, t[:, np.newaxis])))

        # check convergence
        if np.linalg.norm(t) < convergence_translation_threshold and math.acos((np.trace(R) - 1) / 2) < convergence_rotation_threshold:
            if verbose:
                print('Converged!')
            break

    return transformation_history, source_points

reference_points = np.array(numpy_datasets[0][-1][8:16])
points_to_be_aligned = np.array(numpy_datasets[1][0][24:32])

print(reference_points)
print(points_to_be_aligned)

# run icp
transformation_history, aligned_points = icp(reference_points, points_to_be_aligned)

#print(transformation_history)

flatten_dataset_1 = numpy_datasets[1].reshape(-1, 2)
flatten_dataset_2 = numpy_datasets[2].reshape(-1, 2)
flatten_dataset_3 = numpy_datasets[3].reshape(-1, 2)

aligned_points_iter_1 = flatten_dataset_1.copy()  # initialize aligned points with the original points
aligned_points_iter_2 = flatten_dataset_2.copy()
aligned_points_iter_3 = flatten_dataset_3.copy()

for i, transformation in enumerate(transformation_history):
    rot = transformation[:, :2]
    translation = transformation[:, 2]
    aligned_points_iter_1 = np.dot(aligned_points_iter_1, rot.T) + translation
    aligned_points_iter_2 = np.dot(aligned_points_iter_2, rot.T) + translation
    aligned_points_iter_3 = np.dot(aligned_points_iter_3, rot.T) + translation

    x_coords = numpy_datasets[0][:, :, 0].flatten()
    y_coords = numpy_datasets[0][:, :, 1].flatten()

    plt.figure()
    
    plt.scatter(x_coords, y_coords, color='blue', marker='.')
    plt.scatter(aligned_points_iter_1[:, 0], aligned_points_iter_1[:, 1], color='blue', marker='.')
    plt.scatter(aligned_points_iter_2[:, 0], aligned_points_iter_2[:, 1], color='blue', marker='.')
    plt.scatter(aligned_points_iter_3[:, 0], aligned_points_iter_3[:, 1], color='blue', marker='.')

    plt.xlabel('X Coordinates')
    plt.ylabel('Y Coordinates')
    plt.title(f'ICP Iteration {i+1}')
    plt.legend()
    plt.grid(True)
    plt.axis('equal') 
    plt.show()

aligned_points_iter_1_saved = aligned_points_iter_1.copy()    

#RUN 2

reference_points = np.array(numpy_datasets[1][-1][8:16])
points_to_be_aligned = np.array(numpy_datasets[2][0][24:32])
print(reference_points)
print(points_to_be_aligned)

# run icp
transformation_history, aligned_points = icp(reference_points, points_to_be_aligned, verbose=True)

#print(transformation_history)

flatten_dataset_2 = numpy_datasets[2].reshape(-1, 2)
flatten_dataset_3 = numpy_datasets[3].reshape(-1, 2)

aligned_points_iter_2 = flatten_dataset_2.copy()
aligned_points_iter_3 = flatten_dataset_3.copy()

for i, transformation in enumerate(transformation_history):
    rot = transformation[:, :2]
    translation = transformation[:, 2]
    aligned_points_iter_2 = np.dot(aligned_points_iter_2, rot.T) + translation
    aligned_points_iter_3 = np.dot(aligned_points_iter_3, rot.T) + translation

    x_coords = numpy_datasets[0][:, :, 0].flatten()
    y_coords = numpy_datasets[0][:, :, 1].flatten()

    plt.figure()
    
    plt.scatter(x_coords, y_coords, color='blue', marker='.')
    plt.scatter(aligned_points_iter_1_saved[:, 0], aligned_points_iter_1[:, 1], color='blue', marker='.')
    plt.scatter(aligned_points_iter_2[:, 0], aligned_points_iter_2[:, 1], color='blue', marker='.')
    plt.scatter(aligned_points_iter_3[:, 0], aligned_points_iter_3[:, 1], color='blue', marker='.')

    plt.xlabel('X Coordinates')
    plt.ylabel('Y Coordinates')
    plt.title(f'ICP Iteration {i+1}')
    plt.legend()
    plt.grid(True)
    plt.axis('equal') 
    plt.show()

aligned_points_iter_2_saved = aligned_points_iter_2.copy()   

#RUN 3

reference_points = np.array(numpy_datasets[2][-1][8:16])
points_to_be_aligned = np.array(numpy_datasets[3][0][24:32])
print(reference_points)
print(points_to_be_aligned)

# run icp
transformation_history, aligned_points = icp(reference_points, points_to_be_aligned, verbose=True)

#print(transformation_history)

flatten_dataset_3 = numpy_datasets[3].reshape(-1, 2)

aligned_points_iter_3 = flatten_dataset_3.copy()

for i, transformation in enumerate(transformation_history):
    rot = transformation[:, :2]
    translation = transformation[:, 2]
    aligned_points_iter_3 = np.dot(aligned_points_iter_3, rot.T) + translation

    x_coords = numpy_datasets[0][:, :, 0].flatten()
    y_coords = numpy_datasets[0][:, :, 1].flatten()

    plt.figure()
    
    plt.scatter(x_coords, y_coords, color='blue', marker='.')
    plt.scatter(aligned_points_iter_1_saved[:, 0], aligned_points_iter_1[:, 1], color='blue', marker='.')
    plt.scatter(aligned_points_iter_2_saved[:, 0], aligned_points_iter_2[:, 1], color='blue', marker='.')
    plt.scatter(aligned_points_iter_3[:, 0], aligned_points_iter_3[:, 1], color='blue', marker='.')

    plt.xlabel('X Coordinates')
    plt.ylabel('Y Coordinates')
    plt.title(f'ICP Iteration {i+1}')
    plt.legend()
    plt.grid(True)
    plt.axis('equal') 
    plt.show()

aligned_points_iter_3_saved = aligned_points_iter_3.copy()   

# Function to rotate and transform points
def rotate_and_transform_points(x, y, angle_degrees, translate_x, translate_y):
    # Calculate the centroid of the points
    centroid_x = np.mean(x)
    centroid_y = np.mean(y)
    
    # Shift the points to the origin (centroid as origin)
    x_shifted = x - centroid_x
    y_shifted = y - centroid_y
    
    # Convert the angle to radians
    angle_radians = np.radians(angle_degrees)
    
    # Define the rotation matrix
    rotation_matrix = np.array([
        [np.cos(angle_radians), -np.sin(angle_radians)],
        [np.sin(angle_radians), np.cos(angle_radians)]
    ])
    
    # Apply the rotation matrix to the points
    points = np.vstack((x_shifted, y_shifted))
    rotated_points = rotation_matrix @ points
    
    # Shift the points back to the original position and apply translation
    x_rotated = rotated_points[0] + centroid_x + translate_x
    y_rotated = rotated_points[1] + centroid_y + translate_y
    
    return x_rotated, y_rotated





