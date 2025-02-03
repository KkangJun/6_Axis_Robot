package com.example.a6_axis_app

import android.graphics.Color
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.LinearLayout
import android.widget.TextView
import androidx.constraintlayout.widget.ConstraintLayout
import androidx.recyclerview.widget.RecyclerView
import java.util.Locale

data class BoardItem(val name: String, val pos1: Int, val pos2: Int, val pos3: Int,val pos4: Int,val pos5: Int, val pos6: Int)

class BoardAdapter(val itemList: ArrayList<BoardItem>) :
    RecyclerView.Adapter<BoardAdapter.BoardViewHolder>() {
        var selectedItemPosition = -1
        var selectedLayout: ConstraintLayout? = null

        override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): BoardViewHolder {
            val view = LayoutInflater.from(parent.context).inflate(R.layout.item_recycler_view, parent, false)
            return BoardViewHolder(view)
        }

        override fun onBindViewHolder(holder: BoardViewHolder, position: Int) {
            val itemLayout = holder.itemView.findViewById<ConstraintLayout>(R.id.itemLy)
            holder.tv_name.text = itemList[position].name
            holder.tv_pos1.text = String.format(Locale.US, "%d", itemList[position].pos1)
            holder.tv_pos2.text = String.format(Locale.US, "%d", itemList[position].pos2)
            holder.tv_pos3.text = String.format(Locale.US, "%d", itemList[position].pos3)
            holder.tv_pos4.text = String.format(Locale.US, "%d", itemList[position].pos4)
            holder.tv_pos5.text = String.format(Locale.US, "%d", itemList[position].pos5)
            holder.tv_pos6.text = String.format(Locale.US, "%d", itemList[position].pos6)

            if (position == selectedItemPosition) {
                itemLayout.setBackgroundColor(Color.parseColor("#89FFD9"))
            } else {
                itemLayout.setBackgroundColor(Color.parseColor("#E8FFF9"))
            }

            itemLayout.setOnClickListener {
                val currentPosition = holder.adapterPosition

                if (selectedItemPosition == currentPosition) {
                    selectedItemPosition = -1
                    selectedLayout?.setBackgroundColor(Color.parseColor("#E8FFF9"))
                    selectedLayout = null
                } else {
                    if (selectedItemPosition >= 0 || selectedLayout != null) {
                        selectedLayout?.setBackgroundColor(Color.parseColor("#E8FFF9"))
                    }

                    selectedItemPosition = currentPosition
                    selectedLayout = itemLayout
                    selectedLayout?.setBackgroundColor(Color.parseColor("#89FFD9"))
                }
            }
        }

        override fun getItemCount(): Int {
            return itemList.count()
        }

        fun getSelectedItem(): Int? {
            return if (selectedItemPosition >= 0)
                selectedItemPosition
            else null
        }

        inner class BoardViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
            val tv_name = itemView.findViewById<TextView>(R.id.tv_name)
            val tv_pos1 = itemView.findViewById<TextView>(R.id.tv_pos1)
            val tv_pos2 = itemView.findViewById<TextView>(R.id.tv_pos2)
            val tv_pos3 = itemView.findViewById<TextView>(R.id.tv_pos3)
            val tv_pos4 = itemView.findViewById<TextView>(R.id.tv_pos4)
            val tv_pos5 = itemView.findViewById<TextView>(R.id.tv_pos5)
            val tv_pos6 = itemView.findViewById<TextView>(R.id.tv_pos6)
        }
}