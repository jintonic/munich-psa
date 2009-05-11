void drawgdml(char *filename="test.gdml") 
{
   gSystem->Load("libGeom");
   
   gSystem->Load("libGdml");
   
   TGeoManager::Import(filename);
   
   gGeoManager->GetTopVolume()->Draw();
}

