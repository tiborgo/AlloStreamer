
1. Optimized Textures and Materials
--------------------------------------------
Please note that all tree prefabs share the same materials and textures in order to save texture memory and texture load at runtime.

Usually unity automaticly creates materials and textures for each tree.
I have just deleted all those texture folders except for the first one, then assigned the materials "Optimized Bark Metrial" and "Optimized Leaf Material" both located in the Prefab "xyz__donnotdelete" to each tree by dragging them from the project tab to the proper slot of each tree in the inspector tab.

Changing any parameter on any tree will unity force to recalculate the textures so the additional folders will be recreated. In this case just delete the new folders and reassign the original materials like described above.

Those materials use the textures you find in the folder "xyz__donnotdelete_Textures". Please note the special texture settings of those textures: High Aniso Level [9] on the diffuse texture and kaiser-mipmapping.


