// Copyright 2011 - NewTek, Inc. All rights reserved
// This file contains confidential and proprietary information of Newtek, Inc.
// and is subject to the terms of the LightWave EULA.

using UnityEngine;
using UnityEditor;
using System.Collections;
using System.IO;
using System.Diagnostics;
using System.Linq;
using Microsoft.Win32;

public class ChooseLightWavePath : MonoBehaviour
{
    [MenuItem("LightWave/Choose LightWave Install Path")]
    static void DoSomething()
    {
        LWSPostProcessor.query_and_write_lwsn_path();
    }
}

public class LWSPostProcessor : AssetPostprocessor
{
	private static string lw_unity_key_path = @"SOFTWARE\NewTek\LightWave\11.0\Unity3D";
	private static string lw_unity_value_name = "lwsn_path";
		
    private static string read_lwsn_path ()
    {
    	RegistryKey LocalMachineKey = Microsoft.Win32.Registry.LocalMachine;
    	RegistryKey lightwave_key = LocalMachineKey.OpenSubKey (lw_unity_key_path);
    	if (lightwave_key == null)
    		return ""; 
		if (!lightwave_key.GetValueNames().Contains(lw_unity_value_name))
    		return "";
    	return (string)lightwave_key.GetValue (lw_unity_value_name);
	}
	
	private static void write_lwsn_path (string value)
	{
		RegistryKey LocalMachineKey = Microsoft.Win32.Registry.LocalMachine;
		RegistryKey lw_unity_key = LocalMachineKey.CreateSubKey (lw_unity_key_path);
		lw_unity_key.SetValue (lw_unity_value_name, value, RegistryValueKind.String);
		lw_unity_key.Flush ();	
	}
	
	public static string query_and_write_lwsn_path ()
	{
		string lw_path = EditorUtility.OpenFolderPanel("Specify the LightWave installation path", "", "");
		string lwsn_path = lw_path + "/bin/";
		if (Application.platform == RuntimePlatform.OSXEditor)
			lwsn_path = Path.Combine(lwsn_path, "lwsn");
		else
			lwsn_path = Path.Combine(lwsn_path, "lwsn.exe");

		write_lwsn_path (lwsn_path);
		return lwsn_path;
	}
	
	private static string get_lwsn_path ()
	{
		string lwsn_path = read_lwsn_path ();
		if (lwsn_path.Length == 0)
			lwsn_path = query_and_write_lwsn_path ();
		else
			if (!File.Exists (lwsn_path))
				lwsn_path = query_and_write_lwsn_path ();
		
		return lwsn_path;
	}
	
    private static bool is_scene_dir(string lws_dir)
    {
        return Path.GetFileName(lws_dir) == "scenes" || Path.GetFileName(lws_dir) == "Scenes";
    }

    private static string GuessContentDirectory(string lws_dir)
    {
        // simply get the parent directory until a "Scenes" or "scenes" directory is found, and use its parent	
        while (!is_scene_dir(lws_dir))
            lws_dir = Directory.GetParent(lws_dir).ToString();

        if (is_scene_dir(lws_dir))
            return Directory.GetParent(lws_dir).ToString();
        else
            return "";
    }

    private static void OnPostprocessAllAssets(string[] importedAssets, string[] deletedAssets, string[] movedAssets, string[] movedFromPath)
    {
        foreach (string str in importedAssets)
        {
            // UnityEngine.Debug.Log("LW_Imported Asset: " + str);
            string ext = Path.GetExtension(str);
            if (ext == ".lws")
            {
                string project_path = Directory.GetParent(Application.dataPath).ToString();
                string asset_base = Path.GetDirectoryName(str);
                string asset_path = Path.Combine(project_path + "/", asset_base);
                string lws_path = Path.Combine(project_path + "/", str);
                string content_directory = GuessContentDirectory(asset_path);
                if (content_directory.Length == 0)
                    content_directory = asset_path;
                string fbx_name = Path.GetFileNameWithoutExtension(str) + ".fbx";
                string fbx_path = Path.Combine(asset_path + "/", fbx_name);
				
				string lwsn_path = get_lwsn_path();

                Process lwsn = new Process();
                lwsn.StartInfo.WorkingDirectory = Path.GetDirectoryName(lwsn_path);
                lwsn.StartInfo.FileName = lwsn_path;
                lwsn.StartInfo.Arguments = "-5 -d\"" + content_directory + "\" \"LoadScene " + lws_path + "\" \"Generic_ExportFBXCommand '" + fbx_path + "'\"";
                lwsn.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
                lwsn.Start();
                // wait until the FBX file is written
                lwsn.WaitForExit();

                // force a re-load of the FBX
                string fbx_asset_path = Path.Combine(Path.GetDirectoryName(str) + "/", fbx_name);
                AssetDatabase.ImportAsset(fbx_asset_path);
            }
        }
    }
}