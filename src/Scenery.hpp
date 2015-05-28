float randNumInRange(int min, int max) {
   return (rand()%(max-min))+min;
}

vector<Entity> generateScenery(vector<int> &types, Object *obj, Terrain *terrain){
   int size = terrain->getWidth();   
   int cellSize = size/CELL;
   int xMin, xMax, zMin, zMax;
   int height = 0;
   int x, z;   
   vector<Entity> props;   
   Entity dumb;   

   cout << "Welcome Scene\n";

   for(int i = 0; i < CELL; i++) {
      xMin = cellSize * i;                  
      xMax = cellSize * (i+1);

      xMax -= 1;
      xMin += 1;

      for(int j = 0; j < CELL; j++){
         zMin = cellSize * j;
         zMax = cellSize * (j+1);
         
         zMax -= 1;
         zMin += 1;
         
         x = randNumInRange(xMin, xMax); 
         z = randNumInRange(zMin, zMax); 
         cout << "A\n";
         
         cout << "xMin: " << xMin << endl;
         cout << "xMax: " << xMax << endl;         
         cout << "zMin: " << zMin << endl;
         cout << "zMax: " << zMax << endl;
         cout << "x: " << x << endl;
         cout << "z: " << z << endl;

         cout << "Width: " << terrain->getWidth() << endl;
         cout << "Index: " << z * terrain->getWidth() + x << endl;
         height = terrain->heights[(int) z * terrain->getWidth() + (int) x];         
         Eigen::Vector3f normal = terrain->normals[(int) z * terrain->getWidth() + (int) x];
         
         cout << "height: " << height << endl;         
         int dotComp = normal.dot(Eigen::Vector3f(0.0f, 1.0f, 0.0f));         
         cout << "DOT: " << dotComp << endl;         
         int type = rand() % 11;
 
         if(height < 7 && height > .5 && (dotComp > 2 || type < 6)){ 
            Entity prop = Entity();
            
            if(type >= 6){ 
               type = 6;
               int treeScale = randNumInRange(6.0, 8.0);
               int treeYScale = randNumInRange(7.0, 9.0);
               prop.setScale(glm::vec3(treeScale, treeScale, treeScale));
            }
            else{
               int scale = randNumInRange(2.0, 3.0);
               prop.setScale(glm::vec3(scale, scale, scale));
            }

            prop.setObject(&obj[5+type]);
            prop.setPosition(glm::vec3(x, height, z));
            //prop.setScale(glm::vec3(2.0, 2.0, 2.0));
            prop.setMaterial(Materials::emerald);
            
            //if(type == 0){
            //}
            //else if(type == 1){
            //   prop.setObject(&obj[6]);
            //   propIndices = initVBO(&prop, TREE);
            //}            

            prop.calculateBoundingSphereRadius();
            props.push_back(prop);
            types.push_back(type);
         }
         cout << j << endl;
      }  
   }  
   cout << "Leaving SCENE\n"; 
   cout << "Props: " << props.size() << endl;
   return props;
}
