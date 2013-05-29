package com.seeedstudio.nfcdemo;

import java.nio.ByteBuffer;

import android.app.ActionBar;
import android.content.Intent;
import android.nfc.NdefMessage;
import android.nfc.NdefRecord;
import android.nfc.NfcAdapter;
import android.nfc.NfcAdapter.CreateNdefMessageCallback;
import android.nfc.NfcEvent;
import android.os.Bundle;
import android.os.Parcelable;
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import android.view.ViewGroup.LayoutParams;
import android.widget.ArrayAdapter;
import android.widget.FrameLayout;
import android.widget.TextView;
import android.widget.Toast;

public class SenseActivity extends FragmentActivity implements
		ActionBar.OnNavigationListener, CreateNdefMessageCallback {

	/**
	 * The serialization (saved instance state) Bundle key representing the
	 * current dropdown position.
	 */
	private static final String STATE_SELECTED_NAVIGATION_ITEM = "selected_navigation_item";

	private static final String TAG = "NFC Demo";
	private NfcAdapter mNfcAdapter;
	private FrameLayout mChartContainer;
	private LineChartView mChartView;
	private TextView mHumidityView;
	private TextView mHeartRateView;
	private String mOutputString;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.sense);

		mHumidityView = (TextView) findViewById(R.id.humidityView);
		mHeartRateView = (TextView) findViewById(R.id.heartRateView);

		mChartContainer = (FrameLayout) findViewById(R.id.chartContainer);
		mChartView = new LineChartView(this, new double[] { 24.2, 44, 36, 10,
				26.1, 23.6, 20.3, 17.2, 13.9, 12.3, 12.5, 13.8, 16.8, 20.4,
				24.4, 11, 4, 29, 40 }, "Temperature");
		mChartContainer.addView(mChartView.getLineChartView(),
				new LayoutParams(LayoutParams.MATCH_PARENT,
						LayoutParams.MATCH_PARENT));

		// Set up the action bar to show a dropdown list.
		final ActionBar actionBar = getActionBar();
		actionBar.setDisplayShowTitleEnabled(false);
		actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_LIST);

		// Set up the dropdown list navigation in the action bar.
		actionBar.setListNavigationCallbacks(
		// Specify a SpinnerAdapter to populate the dropdown list.
				new ArrayAdapter<String>(actionBar.getThemedContext(),
						android.R.layout.simple_list_item_1,
						android.R.id.text1, new String[] {
								getString(R.string.sense),
								getString(R.string.write) }), this);

		mNfcAdapter = NfcAdapter.getDefaultAdapter(this);
		if (mNfcAdapter == null) {
			Toast.makeText(this, "NFC is not available", Toast.LENGTH_LONG)
					.show();
			finish();
			return;
		}
		// Register callback
		mNfcAdapter.setNdefPushMessageCallback(this, this);
	}

	@Override
	public NdefMessage createNdefMessage(NfcEvent event) {
		byte[] textBytes = mOutputString.getBytes();
		NdefRecord textRecord = new NdefRecord(NdefRecord.TNF_MIME_MEDIA,
				"text/plain".getBytes(), new byte[] {}, textBytes);
		return new NdefMessage(new NdefRecord[] { textRecord });
	}

	@Override
	public void onResume() {
		super.onResume();
		// Check to see that the Activity started due to an Android Beam
		if (NfcAdapter.ACTION_NDEF_DISCOVERED.equals(getIntent().getAction())) {
			processIntent(getIntent());
		} else {
			mOutputString = "Temperature: 40\nHumidity: 56%\nHeart Rate: 73";
		}
	}

	@Override
	public void onNewIntent(Intent intent) {
		// onResume gets called after this to handle the intent
		setIntent(intent);
	}

	/**
	 * Parses the NDEF Message from the intent and show the content
	 */
	void processIntent(Intent intent) {
		Parcelable[] rawMsgs = intent
				.getParcelableArrayExtra(NfcAdapter.EXTRA_NDEF_MESSAGES);

		if (rawMsgs.length != 1) {
			return;
		}

		NdefMessage msg = (NdefMessage) rawMsgs[0];
		NdefRecord[] records = msg.getRecords();

		if (records.length != 4) {
			return;
		}

		// temperature
		String temperaturesString = new String(records[0].getPayload());
		Log.v(TAG, "Temperature: " + temperaturesString);
		String temperatureString[] = temperaturesString.split(" ");
		mOutputString = "Temperature: "
				+ temperatureString[temperatureString.length - 1];
		double[] temperature = new double[temperatureString.length];
		for (int i = 0; i < temperature.length; i++) {
			try {
				temperature[i] = Double.parseDouble(temperatureString[i]);
			} catch (NumberFormatException e) {
				temperature[i] = 0;
			}
		}
		mChartContainer.removeAllViews();
		mChartContainer.addView(mChartView.refreshChart(temperature),
				new LayoutParams(LayoutParams.MATCH_PARENT,
						LayoutParams.MATCH_PARENT));

		// humidity
		String humidityString = new String(records[1].getPayload());
		Log.v(TAG, "Humidity: " + humidityString);
		mHumidityView.setText(humidityString);
		mOutputString += "\nHumidity: " + humidityString;

		// heart rate
		String rateString = new String(records[2].getPayload());
		Log.v(TAG, "Heart Rate: " + rateString);
		double rate;
		try {
			rate = Double.parseDouble(rateString);
		} catch (NumberFormatException e) {
			rate = -1;
		}

		if (Double.compare(rate, -1) == 0) {
			mHeartRateView.setText("~");
		} else {
			mHeartRateView.setText(rateString);
			mOutputString += "\nHeart Rate: " + rateString;
		}
	}

	@Override
	public void onRestoreInstanceState(Bundle savedInstanceState) {
		// Restore the previously serialized current dropdown position.
		if (savedInstanceState.containsKey(STATE_SELECTED_NAVIGATION_ITEM)) {
			getActionBar().setSelectedNavigationItem(
					savedInstanceState.getInt(STATE_SELECTED_NAVIGATION_ITEM));
		}
	}

	@Override
	public void onSaveInstanceState(Bundle outState) {
		// Serialize the current dropdown position.
		outState.putInt(STATE_SELECTED_NAVIGATION_ITEM, getActionBar()
				.getSelectedNavigationIndex());
	}

	@Override
	public boolean onNavigationItemSelected(int position, long id) {
		// When the given dropdown item is selected, show its contents in the
		// container view.
		Log.v(TAG, "Position: " + position + ",ID: " + id);
		if (position == 1) {
			Intent intent = new Intent(this, WriteActivity.class);
			startActivity(intent);
			finish();
		}
		return true;
	}

}
