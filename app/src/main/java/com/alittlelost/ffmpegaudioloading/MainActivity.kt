package com.alittlelost.ffmpegaudioloading

import android.content.Context
import android.content.res.AssetManager
import android.os.Bundle
import android.system.Os
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import com.alittlelost.ffmpegaudioloading.databinding.ActivityMainBinding
import java.io.File

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        copyWavToFilesDir()
        Os.setenv("FILES_DIR_PATH", filesDir.absolutePath, true)

        // Example of a call to a native method
        binding.sampleText.text = stringFromJNI()
    }

    private fun copyWavToFilesDir() {
        if (doesWavExist()) {
            Log.i("Main", "metro.wav already exists in filesDir")
            return
        }

        resources.openRawResource(R.raw.metro).use { input ->
            openFileOutput("metro.wav", Context.MODE_PRIVATE).use { output ->
                input.copyTo(output)
            }
        }

        if(doesWavExist()) {
            Log.i("Main", "Successfully copied wav to files dir")
        } else {
            Log.e("Main", "Copying did nae work")
        }

    }

    private fun doesWavExist(): Boolean {
        val file = File(filesDir, "metro.wav")
        return file.exists()
    }

    /**
     * A native method that is implemented by the 'ffmpegaudioloading' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    companion object {
        // Used to load the 'ffmpegaudioloading' library on application startup.
        init {
            System.loadLibrary("ffmpegaudioloading")
        }
    }
}