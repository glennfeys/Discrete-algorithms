# Common code

## Parser

De parser werkt als volgt:
```cpp
DummyGraph graph; // Of een ander graaf type, de dummy print gewoon de edges uit die hij binnenkrijgt.
ClqParser parser;
parser.parse(*"/home/niels/Downloads/C125.9.clq", graph);
```

Het graaftype moet de volgende twee methoden hebben:
- ```void setParameters(uint32_t numberOfVertices, uint32_t numberOfEdges)```
- ```void addEdge(uint32_t from, uint32_t to)```
