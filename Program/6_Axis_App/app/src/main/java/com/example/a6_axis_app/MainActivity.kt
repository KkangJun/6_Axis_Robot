package com.example.a6_axis_app

import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothSocket
import android.content.Context
import android.content.DialogInterface
import android.content.pm.PackageManager
import android.graphics.Color
import android.os.Build
import android.os.Bundle
import android.text.Editable
import android.text.TextWatcher
import android.util.Log
import android.widget.Button
import android.widget.EditText
import android.widget.SeekBar
import androidx.activity.enableEdgeToEdge
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.example.a6_axis_app.databinding.ActivityMainBinding
import com.example.a6_axis_app.databinding.AlertdialogEdittextBinding
import android.widget.Toast
import com.google.gson.Gson
import com.google.gson.reflect.TypeToken
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch
import java.util.Locale
import java.util.UUID

class MainActivity : AppCompatActivity() {
    val itemList = ArrayList<BoardItem>()
    var item_idx = itemList.count()

    var bluetoothSocket: BluetoothSocket? = null
    var bluetoothAdapter: BluetoothAdapter? = null
    var hc05Device: BluetoothDevice? = null

    var isRunMotor = false

    var npos1 = 180
    var npos2 = 0
    var npos3 = 0
    var npos4 = 17
    var npos5 = 180
    var npos6 = 180

    lateinit var binding : ActivityMainBinding

    private var job: Job? = null

    private fun start() {
        // initialize
        npos1 = 180
        npos2 = 0
        npos3 = 0
        npos4 = 17
        npos5 = 180
        npos6 = 180

        setContentView(R.layout.activity_start)

        val btn_con = findViewById<Button>(R.id.btnCon)

        val bluetoothManager = getSystemService(Context.BLUETOOTH_SERVICE) as? BluetoothManager
        bluetoothAdapter = bluetoothManager?.adapter

        // Connect to Bluetooth
        btn_con.setOnClickListener {
            if (bluetoothAdapter == null || !bluetoothAdapter!!.isEnabled) {
                Toast.makeText(this@MainActivity, "Please enable Bluetooth", Toast.LENGTH_SHORT)
                    .show()
            }

            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                if (checkSelfPermission(android.Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED ||
                    checkSelfPermission(android.Manifest.permission.BLUETOOTH_ADMIN) != PackageManager.PERMISSION_GRANTED) {
                    requestPermissions(arrayOf(android.Manifest.permission.BLUETOOTH_CONNECT, android.Manifest.permission.BLUETOOTH_ADMIN), 1)
                }
            }

            hc05Device = bluetoothAdapter!!.bondedDevices.find { it.name == "HC-05" }
            if (hc05Device == null) {
                Toast.makeText(this@MainActivity, "HC-05 device not paired. Please pair it first.", Toast.LENGTH_SHORT)
                    .show()
            }

            try {
                bluetoothSocket = connectToDevice(hc05Device!!)
                if (bluetoothSocket!!.isConnected) {
                    val outputStream = bluetoothSocket!!.outputStream

                    outputStream.write("\n".toByteArray())
                    val str = "RSET\r\n"
                    outputStream.write(str.toByteArray())

                    Toast.makeText(this@MainActivity, "Robot connect", Toast.LENGTH_SHORT)
                        .show()
                    main()
                } else {
                    Toast.makeText(this@MainActivity, "Unable to connect to HC-05", Toast.LENGTH_SHORT)
                        .show()
                }
            } catch (e: Exception) {
                Toast.makeText(this@MainActivity, "Error connecting to HC-05", Toast.LENGTH_SHORT)
                    .show()
            }
        }
    }

    private fun main() {
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        val btn_dcon = findViewById<Button>(R.id.btnDcon)
        val btn_pos_list = findViewById<Button>(R.id.btnPosList)
        val btn_run_stop = findViewById<Button>(R.id.btnRunStop)
        val btn_reset = findViewById<Button>(R.id.btnReset)

        val outputStream = bluetoothSocket!!.outputStream

        var saveFlag = false

        val sbar_pos1 = findViewById<SeekBar>(R.id.sbarPos1)
        val sbar_pos2 = findViewById<SeekBar>(R.id.sbarPos2)
        val sbar_pos3 = findViewById<SeekBar>(R.id.sbarPos3)
        val sbar_pos4 = findViewById<SeekBar>(R.id.sbarPos4)
        val sbar_pos5 = findViewById<SeekBar>(R.id.sbarPos5)
        val sbar_pos6 = findViewById<SeekBar>(R.id.sbarPos6)

        val edt_pos1 = findViewById<EditText>(R.id.edtPos1)
        val edt_pos2 = findViewById<EditText>(R.id.edtPos2)
        val edt_pos3 = findViewById<EditText>(R.id.edtPos3)
        val edt_pos4 = findViewById<EditText>(R.id.edtPos4)
        val edt_pos5 = findViewById<EditText>(R.id.edtPos5)
        val edt_pos6 = findViewById<EditText>(R.id.edtPos6)

        // initialize
        sbar_pos1.setProgress(npos1, true)
        sbar_pos2.setProgress(npos2, true)
        sbar_pos3.setProgress(npos3, true)
        sbar_pos4.setProgress(npos4, true)
        sbar_pos5.setProgress(npos5, true)
        sbar_pos6.setProgress(npos6, true)

        edt_pos1.setText("${npos1}")
        edt_pos2.setText("${npos2}")
        edt_pos3.setText("${npos3}")
        edt_pos4.setText("${npos4}")
        edt_pos5.setText("${npos5}")
        edt_pos6.setText("${npos6}")

        // Sync SeekBar & EditText
        inputControl(sbar_pos1, edt_pos1, "P1")
        inputControl(sbar_pos2, edt_pos2, "P2")
        inputControl(sbar_pos3, edt_pos3, "P3")
        inputControl(sbar_pos4, edt_pos4, "P4")
        inputControl(sbar_pos5, edt_pos5, "P5")
        inputControl(sbar_pos6, edt_pos6, "P6")

        job = CoroutineScope(Dispatchers.IO).launch {
            while (isActive) {
                if(saveFlag) {
                    val str = String.format(Locale.US, "SAV.%d.%03d.%03d.%03d.%03d.%03d.%03d\r\n", item_idx - 1, npos1, npos2, npos3, npos4, npos5, npos6)
                    outputStream.write(str.toByteArray())

                    saveFlag = false
                }
                delay(100)
            }
        }

        // Disconnect to Bluetooth
        btn_dcon.setOnClickListener {
            val str = "STOP\r\n"
            outputStream.write(str.toByteArray())

            isRunMotor = false

            bluetoothSocket?.close()
            start()
            Toast.makeText(this@MainActivity, "Robot Disconnect", Toast.LENGTH_SHORT)
                .show()
        }

        // Save Motor Position (with dialog)
        binding.btnSavePos.setOnClickListener {
            val builder = AlertDialog.Builder(this)
            val builderItem = AlertdialogEdittextBinding.inflate(layoutInflater)
            val dialog_edt = builderItem.editText

            if(itemList.count() < 5) {
                with(builder) {
                    setTitle("Save motor position")
                    setMessage("Please check the position again")
                    setView(builderItem.root)
                    setPositiveButton("Save") { dialogInterface: DialogInterface, i: Int ->
                        val name = dialog_edt.text.toString()
                        if (name != "") {
                            itemList.add(
                                BoardItem(
                                    name, sbar_pos1.progress, sbar_pos2.progress, sbar_pos3.progress,
                                    sbar_pos4.progress, sbar_pos5.progress, sbar_pos6.progress
                                )
                            )
                            saveFlag = true
                            item_idx = itemList.count()
                            Toast.makeText(this@MainActivity, "Complete save", Toast.LENGTH_SHORT).show()
                        } else {
                            Toast.makeText(
                                this@MainActivity,
                                "Error: Please check name",
                                Toast.LENGTH_SHORT
                            ).show()
                        }
                    }
                    setNegativeButton("Cancel") { dialogInterface: DialogInterface, i: Int ->
                        Toast.makeText(this@MainActivity, "Cancel save", Toast.LENGTH_SHORT).show()
                    }
                    show()
                }
            } else {
                Toast.makeText(this@MainActivity, "You cannot save more than 5", Toast.LENGTH_SHORT).show()
            }
        }

        // Go to Position List
        btn_pos_list.setOnClickListener {
            posList()
        }

        // Run & Stop Motor
        btn_run_stop.setOnClickListener {
            if (isRunMotor) {
                val str = "STOP\r\n"
                outputStream.write(str.toByteArray())

                btn_run_stop.setText("Run")
                btn_run_stop.setBackgroundColor(Color.parseColor("#4075BA"))

                Toast.makeText(this@MainActivity, "Stop Motors", Toast.LENGTH_SHORT)
                    .show()

                isRunMotor = false
            } else {
                val str = String.format(Locale.US, "SET..%03d.%03d.%03d.%03d.%03d.%03d\r\n", npos1, npos2, npos3, npos4, npos5, npos6)
                outputStream.write(str.toByteArray())

                btn_run_stop.setText("Stop")
                btn_run_stop.setBackgroundColor(Color.parseColor("#BA4075"))

                Toast.makeText(this@MainActivity, "Run Motors", Toast.LENGTH_SHORT)
                    .show()

                isRunMotor = true
            }
        }

        // Reset Motor Position
        btn_reset.setOnClickListener {
            npos1 = 180
            npos2 = 0
            npos3 = 0
            npos4 = 17
            npos5 = 180
            npos6 = 180

            sbar_pos1.setProgress(npos1, true)
            sbar_pos2.setProgress(npos2, true)
            sbar_pos3.setProgress(npos3, true)
            sbar_pos4.setProgress(npos4, true)
            sbar_pos5.setProgress(npos5, true)
            sbar_pos6.setProgress(npos6, true)

            edt_pos1.setText("${npos1}")
            edt_pos2.setText("${npos2}")
            edt_pos3.setText("${npos3}")
            edt_pos4.setText("${npos4}")
            edt_pos5.setText("${npos5}")
            edt_pos6.setText("${npos6}")

            val str = "RSET\r\n"
            outputStream.write(str.toByteArray())

            Toast.makeText(this@MainActivity, "Reset Motors", Toast.LENGTH_SHORT)
                .show()
        }
    }

    private fun posList() {
        setContentView(R.layout.activity_list)

        val outputStream = bluetoothSocket!!.outputStream

        val btn_back = findViewById<Button>(R.id.btnBack)
        val btn_del = findViewById<Button>(R.id.btnDel)
        val btn_list_set = findViewById<Button>(R.id.btnListSet)
        val btn_list_reset = findViewById<Button>(R.id.btnListReset)

        val rv_board = findViewById<RecyclerView>(R.id.rv_board)
        val boardAdapter = BoardAdapter(itemList)

        boardAdapter.notifyDataSetChanged()

        rv_board.adapter = boardAdapter
        rv_board.layoutManager = LinearLayoutManager(this, LinearLayoutManager.VERTICAL, false)

        // Go to Main
        btn_back.setOnClickListener {
            main()
        }

        // Delete Motor Position
        btn_del.setOnClickListener {
            val idx = boardAdapter.getSelectedItem()

            if (idx != null) {
                val str = String.format(Locale.US, "DEL.%d\r\n", idx)
                outputStream.write(str.toByteArray())

                itemList.removeAt(idx)
                boardAdapter.notifyItemRemoved(idx)
                Toast.makeText(this@MainActivity, "Delete Complete", Toast.LENGTH_SHORT)
                    .show()
            } else {
                Toast.makeText(this@MainActivity, "Error: Please select from the list", Toast.LENGTH_SHORT)
                    .show()
            }
        }

        btn_list_set.setOnClickListener {
            val idx = boardAdapter.getSelectedItem()

            if (idx != null) {
                val str = String.format(Locale.US, "SET..%03d.%03d.%03d.%03d.%03d.%03d\r\n",
                    itemList[idx].pos1, itemList[idx].pos2, itemList[idx].pos3, itemList[idx].pos4, itemList[idx].pos5, itemList[idx].pos6)
                outputStream.write(str.toByteArray())

                Toast.makeText(this@MainActivity, "Set Motors", Toast.LENGTH_SHORT)
                    .show()
            } else {
                Toast.makeText(this@MainActivity, "Error: Please select from the list", Toast.LENGTH_SHORT)
                    .show()
            }
        }

        btn_list_reset.setOnClickListener {
            npos1 = 180
            npos2 = 0
            npos3 = 0
            npos4 = 17
            npos5 = 180
            npos6 = 180

            val str = "RSET\r\n"
            outputStream.write(str.toByteArray())

            Toast.makeText(this@MainActivity, "Reset Motors", Toast.LENGTH_SHORT)
                .show()
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()

        loadItems(itemList)
        start()
    }

    override fun onPause() {
        super.onPause()
        saveItems(itemList)
    }

    override fun onStop() {
        super.onStop()

        job?.cancel()
        if(bluetoothSocket != null) {
            val outputStream = bluetoothSocket!!.outputStream
            val str = "DIS\r\n"
            outputStream.write(str.toByteArray())

            isRunMotor = false

            bluetoothSocket?.close()
        }

        start()
    }

    override fun onDestroy() {
        super.onDestroy()

        if(bluetoothSocket != null) {
            val outputStream = bluetoothSocket!!.outputStream
            val str = "DIS\r\n"
            outputStream.write(str.toByteArray())

            bluetoothSocket?.close()
        }
    }

    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)

        if (requestCode == 1) {
            if (grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED && grantResults[1] == PackageManager.PERMISSION_GRANTED) {
                // 권한 허용됨 -> 다시 start() 호출
                start()
            } else {
                Log.e("BluetoothError", "Bluetooth permissions denied")
                finish() // 권한 없으면 앱 종료
            }
        }
    }

    private fun saveItems(itemList: ArrayList<BoardItem>) {
        val pref = getSharedPreferences("item_pref", MODE_PRIVATE)
        val editor = pref.edit()
        val gson = Gson()
        val json = gson.toJson(itemList)

        editor.putString("pos_items", json)
        editor.apply()
    }

    private fun loadItems(itemList: ArrayList<BoardItem>) {
        val pref = getSharedPreferences("item_pref", MODE_PRIVATE)
        val gson = Gson()
        val json = pref.getString("pos_items", null)

        if (json != null) {
            val type = object: TypeToken<ArrayList<BoardItem>>() {}.type
            itemList.clear()
            itemList.addAll(gson.fromJson(json, type))
        }
    }
    private fun inputControl(seekBar: SeekBar, editText: EditText, pos: String) {
        val outputStream = bluetoothSocket!!.outputStream
        var job: Job? = null

        seekBar.setOnSeekBarChangeListener(
            object: SeekBar.OnSeekBarChangeListener {
                //            @SuppressLint("SetTextI18n")
                override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                    if (fromUser) {
                        when (pos) {
                            "P1" -> npos1 = progress
                            "P2" -> npos2 = progress
                            "P3" -> npos3 = progress
                            "P4" -> npos4 = progress
                            "P5" -> npos5 = progress
                            "P6" -> npos6 = progress
                        }
                        editText.setText("%d".format(progress))

                        if (isRunMotor) {
                            job?.cancel()
                            job = CoroutineScope(Dispatchers.IO).launch {
                                delay(50)
                                val str = pos + "..${progress}\r\n"
                                outputStream.write(str.toByteArray())
                            }
                        }
                    }
                }

                override fun onStartTrackingTouch(seekBar: SeekBar?) {
//                TODO("Not yet implemented")
                }

                override fun onStopTrackingTouch(seekBar: SeekBar?) {
                    var npos = 0
                    when (pos) {
                        "P1" -> npos = npos1
                        "P2" -> npos = npos2
                        "P3" -> npos = npos3
                        "P4" -> npos = npos4
                        "P5" -> npos = npos5
                        "P6" -> npos = npos6
                    }

                    if (isRunMotor) {
                            val str = pos + "..${npos}\r\n"
                            outputStream.write(str.toByteArray())
                }
                }
            }
        )

        editText.addTextChangedListener(
            object: TextWatcher {
                override fun beforeTextChanged(s: CharSequence?, start: Int, count: Int, after: Int) {
//                TODO("Not yet implemented")
                }

                override fun onTextChanged(s: CharSequence?, start: Int, before: Int, count: Int) {
//                TODO("Not yet implemented")
                }

                override fun afterTextChanged(s: Editable?) {
                    try {
                        val progress = (s.toString()).toInt()
                        if ((progress >= 0) && (progress <= seekBar.max)) {
                            when (pos) {
                                "P1" -> npos1 = progress
                                "P2" -> npos2 = progress
                                "P3" -> npos3 = progress
                                "P4" -> npos4 = progress
                                "P5" -> npos5 = progress
                                "P6" -> npos6 = progress
                            }
                            seekBar.setProgress(progress, true)

                            if (isRunMotor) {
                                val str = pos + "..${progress}\r\n"
                                outputStream.write(str.toByteArray())
                            }
                        } else {
                            editText.error = "Value must be between 0 and ${seekBar.max}"
                        }
                    } catch (e: NumberFormatException) {
                        editText.error = "Invalid number"
                    }
                }
            }
        )

        // Action after Enter Key
        editText.setOnEditorActionListener { v, actionId, event ->
            false
        }
    }

    @SuppressLint("MissingPermission")
    private fun connectToDevice(device: BluetoothDevice): BluetoothSocket {
        val uuid = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB")
        val socket = device.createRfcommSocketToServiceRecord(uuid)
        socket.connect()
        return socket
    }
}