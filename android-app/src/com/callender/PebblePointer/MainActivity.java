package com.callender.PebblePointer;

import android.app.ListActivity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;

public class MainActivity extends ListActivity {

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        String[] menu = {
                "Raw Accelerometer Vectors",
        };

        setListAdapter(new ArrayAdapter<String>(this, R.layout.simple_list_item_1, menu));
    }

    @Override
    protected void onListItemClick(final ListView l, final View v, final int position, final long id) {
        Intent intent = null;
        switch (position) {
            case 0:
                intent = new Intent(this, AccelerometerActivity.class);
                break;               
        }

        if (intent != null) {
            startActivity(intent);
        }
    }
}
