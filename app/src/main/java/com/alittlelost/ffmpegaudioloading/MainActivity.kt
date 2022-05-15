package com.alittlelost.ffmpegaudioloading

import android.content.res.AssetManager
import android.media.AudioAttributes
import android.media.AudioFormat
import android.media.AudioTrack
import android.os.Bundle
import android.util.Log
import androidx.annotation.RawRes
import androidx.appcompat.app.AppCompatActivity
import com.alittlelost.ffmpegaudioloading.databinding.ActivityMainBinding
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.launch
import java.io.ByteArrayOutputStream

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    private val minBufferSize = AudioTrack.getMinBufferSize(
        44100,
        AudioFormat.CHANNEL_OUT_STEREO,
        AudioFormat.ENCODING_PCM_16BIT
    )

    private val audioAttributes = AudioAttributes.Builder()
        .setUsage(AudioAttributes.USAGE_MEDIA)
        .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
        .build()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // Example of a call to a native method
        binding.sampleText.text = stringFromJNI()

        Log.i("Main", "MinBufferSize: $minBufferSize")

        if(setAssetManager(getAssetManager())) {
            loadAsset("metro.wav")
        }
    }

//    private fun playRawAudio(@RawRes id: Int) {
//        var buffer = ByteArray(minBufferSize)
//
//        resources.openRawResource(id).use { input ->
//            var i = input.read(buffer)
//            while (i != -1) {
//                i = input.read(buffer)
//            }
//        }
//    }

    private fun getAssetManager() = assets

    /**
     * A native method that is implemented by the 'ffmpegaudioloading' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    external fun setAssetManager(assetManager: AssetManager): Boolean

    external fun loadAsset(assetName: String): Unit

    companion object {
        // Used to load the 'ffmpegaudioloading' library on application startup.
        init {
            System.loadLibrary("ffmpegaudioloading")
        }
    }
}