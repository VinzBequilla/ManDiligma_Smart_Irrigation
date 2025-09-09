
# ManDiligMa

Smart Irrigation and Health Monitoring System for Lettuce Using Machine Learning and Mobile Application

This project demonstrates a smart irrigation system that uses a machine learning model to predict the precise amount of water needed for plant based on environmental sensor readings of Arduino Uno. The system leverages historical data to make intelligent, data-driven decisions, optimizing water usage and promoting sustainable agriculture.

The core of this project is a Python script that trains a Support Vector Regression (SVR) model to predict water requirements. This approach moves beyond simple on/off irrigation rules and provides a more accurate, tailored solution for your plants.

## How It Works
The system operates in two main phases:

**Model Training:** The smart_irrigation_model.py script uses a historical dataset `(svm/TARP.csv from Kaggle)` containing sensor readings for soil moisture, temperature, and humidity. It trains an SVR model to learn the relationship between these environmental factors and a target variable: the actual amount of water needed.

**Real-Time Prediction:** Once the model is trained and saved, it can be deployed on a device (like Arduino Uno or your device itself) connected to real-time sensors. The system will use live sensor readings as input to the trained model, which will then output a prediction for the optimal amount of water to apply.

**Key Features
Data-Driven:** The model learns from your unique environmental data, adapting to your specific conditions.

**Precision:** It predicts the amount of water needed instead of a simple "on/off" decision.

**Data Cleaning:** The script automatically handles missing values in the dataset to ensure a smooth training process.

**Model Persistence:** The trained model is saved to a file `(irrigation_model.pkl)` so you can easily deploy it without retraining.


## Getting Started
Prerequisites

- Python 3.6 or later
- pandas
- scikit-learn
- numpy
- joblib
- pyserial

### You can install these libraries using pip:

``` pip install pandas scikit-learn numpy joblib pyserial ```
