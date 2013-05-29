package com.seeedstudio.nfcdemo;

import java.util.ArrayList;
import java.util.List;

import org.achartengine.ChartFactory;
import org.achartengine.GraphicalView;
import org.achartengine.chart.PointStyle;
import org.achartengine.model.XYMultipleSeriesDataset;
import org.achartengine.model.XYSeries;
import org.achartengine.renderer.XYMultipleSeriesRenderer;
import org.achartengine.renderer.XYSeriesRenderer;

import android.content.Context;
import android.graphics.Color;
import android.graphics.Paint.Align;
import android.util.Log;

public class LineChartView {

	private XYMultipleSeriesDataset mDataset;
	private XYMultipleSeriesRenderer mRenderer;
	private GraphicalView mChartView;

	private Context mContext;
	private double[] mList;
	private String mChartName;

	/**
	 * 构�?函数，初始化�?�� LineChartView 实例�? * @param context 当前的上下文环境�? * @param
	 * mData 图表�?��要的 double 数组
	 * 
	 * @param chartName
	 *            图表名字
	 */
	public LineChartView(Context context, double[] mData, String chartName) {
		this.mContext = context;
		this.mList = mData;
		this.mChartName = chartName;
	}

	/**
	 * 
	 * @return 返回�?��图表试图（view�?
	 */
	public GraphicalView getLineChartView() {
		if (mChartView == null) {
			buildChartView();
			mChartView = ChartFactory.getLineChartView(mContext, mDataset,
					mRenderer);
		}
		return mChartView;

	}

	/**
	 * 刷新图表，因为本身库不支持动态刷新，我直接重新构建了�?��图表试图�? * @param data 同样�?double 的数�? * @return
	 * 返回�?��图表试图（view�?
	 */
	public GraphicalView refreshChart(double[] data) {
		this.mList = data;
		Log.d("Line Chart View", this.mList.toString());
		buildChartView();
		mChartView = ChartFactory.getLineChartView(mContext, mDataset,
				mRenderer);
		return mChartView;
	}

	// build chart view
	protected void buildChartView() {
		// setup Line Chart sytle
		int[] colors = new int[] { Color.RED };
		PointStyle[] styles = new PointStyle[] { PointStyle.CIRCLE };
		mRenderer = buildRenderer(colors, styles);
		int length = mRenderer.getSeriesRendererCount();
		for (int i = 0; i < length; i++) {
			((XYSeriesRenderer) mRenderer.getSeriesRendererAt(i))
					.setFillPoints(true);
		}

		// setup X axis
		String[] titles = new String[] { mChartName };
		List<double[]> x = new ArrayList<double[]>();
		double[] xLength = new double[mList.length];
		for (int i = 0; i < mList.length; i++) {
			xLength[i] = i;
		}
		x.add(xLength);

		// setup Y axis
		List<double[]> values = new ArrayList<double[]>();
		values.add(mList);

		mDataset = buildDataset(titles, x, values);
		// XYSeries series = mDataset.getSeriesAt(0);
		// series.addAnnotation("NFC data", 6, 30); // 添加注释

	}

	// build dataset
	protected XYMultipleSeriesDataset buildDataset(String[] titles,
			List<double[]> xValues, List<double[]> yValues) {
		XYMultipleSeriesDataset dataset = new XYMultipleSeriesDataset();
		addXYSeries(dataset, titles, xValues, yValues, 0);
		return dataset;
	}

	protected void addXYSeries(XYMultipleSeriesDataset dataset,
			String[] titles, List<double[]> xValues, List<double[]> yValues,
			int scale) {
		int length = titles.length;
		for (int i = 0; i < length; i++) {
			XYSeries series = new XYSeries(titles[i], scale);
			double[] xV = xValues.get(i);
			double[] yV = yValues.get(i);
			int seriesLength = xV.length;
			for (int k = 0; k < seriesLength; k++) {
				series.add(xV[k], yV[k]);
			}
			dataset.addSeries(series);
		}
	}

	// build renderer
	protected XYMultipleSeriesRenderer buildRenderer(int[] colors,
			PointStyle[] styles) {
		XYMultipleSeriesRenderer renderer = new XYMultipleSeriesRenderer();
		setRenderer(renderer, colors, styles);
		return renderer;
	}

	protected void setRenderer(XYMultipleSeriesRenderer renderer, int[] colors,
			PointStyle[] styles) {
		renderer.setAxisTitleTextSize(20);
		renderer.setChartTitleTextSize(mContext.getResources()
				.getDimensionPixelSize(R.dimen.item_title_text_size));
		renderer.setLabelsTextSize(20);
		renderer.setLegendTextSize(20);
		renderer.setPointSize(5f);
		renderer.setMargins(new int[] { 60, 60, 8, 20 });
		renderer.setApplyBackgroundColor(true);
		renderer.setBackgroundColor(Color.argb(100, 0, 0, 0));
		renderer.setShowGrid(true);
		renderer.setZoomButtonsVisible(false);
		renderer.setChartTitle(mChartName);
		renderer.setXTitle("t");
		renderer.setYTitle("Temperature(℃）");
		renderer.setAxesColor(Color.LTGRAY);
		renderer.setXLabelsColor(Color.BLACK);
		renderer.setLabelsColor(Color.WHITE);
		renderer.setXLabels(24);
		renderer.setYLabels(20);
		renderer.setXLabelsAlign(Align.RIGHT);
		renderer.setYLabelsAlign(Align.RIGHT);
		renderer.setPanLimits(new double[] { 0, 20, 0, 40 });
		renderer.setZoomLimits(new double[] { 0, 20, 0, 40 });

		int length = colors.length;
		for (int i = 0; i < length; i++) {
			XYSeriesRenderer r = new XYSeriesRenderer();
			r.setColor(colors[i]);
			r.setPointStyle(styles[i]);
			renderer.addSeriesRenderer(r);
		}
	}
}
