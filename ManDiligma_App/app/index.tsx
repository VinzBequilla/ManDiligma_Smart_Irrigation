import React, { useEffect, useRef, useState } from 'react';
import { View, Text, StyleSheet, ScrollView, Dimensions, Animated, TouchableOpacity, Modal, Pressable } from 'react-native';
import { MaterialCommunityIcons } from '@expo/vector-icons';
import { LinearGradient } from 'expo-linear-gradient';
import { initializeApp } from 'firebase/app';
import { getDatabase, ref, onValue } from 'firebase/database';

const { width } = Dimensions.get('window');
const CARD_WIDTH = Math.min(420, width - 32);

// ✅ Firebase Config
const firebaseConfig = {
  databaseURL: 'https://mandiligma-16e1c-default-rtdb.asia-southeast1.firebasedatabase.app',
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);
const db = getDatabase(app);

type SensorCardProps = {
  title: string;
  icon: keyof typeof MaterialCommunityIcons.glyphMap;
  value: number;
  unit?: string;
  min?: number;
  max?: number;
  colorStart: string;
  colorEnd: string;
};

const format = (v: number, unit = '') => `${v}${unit}`;

function SensorCard({ title, icon, value, unit = '', min = 0, max = 100, colorStart, colorEnd }: SensorCardProps) {
  const progress = Math.max(0, Math.min(1, (value - min) / (max - min)));
  const animated = useRef(new Animated.Value(0)).current;

  useEffect(() => {
    Animated.timing(animated, {
      toValue: progress,
      duration: 700,
      useNativeDriver: false,
    }).start();
  }, [progress]);

  const widthInterpolated = animated.interpolate({
    inputRange: [0, 1],
    outputRange: ['4%', '96%'],
  });

  return (
    <LinearGradient colors={[colorStart, colorEnd]} start={[0, 0]} end={[1, 1]} style={styles.cardGradient}>
      <View style={styles.cardRow}>
        <View style={styles.iconWrap}>
          <MaterialCommunityIcons name={icon} size={26} color="#fff" />
        </View>
        <View style={styles.cardContent}>
          <Text style={styles.cardTitle}>{title}</Text>
          <Text style={styles.cardValue}>{format(value, unit)}</Text>
          <View style={styles.miniBarBackground}>
            <Animated.View style={[styles.miniBarFill, { width: widthInterpolated }]} />
          </View>
        </View>
      </View>
    </LinearGradient>
  );
}

type Readings = {
  temperature: number;
  humidity: number;
  soil: number;
  ph: number;
  wateringHistory: string[];
};

export default function SmartIrrigationAppDesign() {
  const [readings, setReadings] = useState<Readings>({
    temperature: 0,
    humidity: 0,
    soil: 0,
    ph: 6.8,
    wateringHistory: [],
  });

  const [modalVisible, setModalVisible] = useState(false);

  useEffect(() => {
  const readingsRef = ref(db, 'readings');

  const unsubscribe = onValue(readingsRef, (snapshot) => {
    const data = snapshot.val();
    console.log('🔥 Raw snapshot from Firebase:', data);

    if (data) {
      const entries: any[] = Object.values(data);
      if (entries.length) {
        // ✅ Just pick the last entry added
        const latest = entries[entries.length - 1];

        const history = entries
          .filter((e) => e.prediction === 1)
          .map((e) => {
            const d = new Date();
            const time = d.toLocaleTimeString('en-US', {
              hour: '2-digit',
              minute: '2-digit',
              hour12: false,
            });
            return `${time}`;
          });

        setReadings({
          temperature: latest.temperature || 0,
          humidity: latest.humidity || 0,
          soil: latest.moisture || 0,
          ph: 6.8,
          wateringHistory: history,
        });

        console.log('✅ Updated Live Data:', latest);
      }
    }
  });

  return () => unsubscribe();
}, []);


  return (
    <ScrollView contentContainerStyle={styles.container} showsVerticalScrollIndicator={false}>
      <Text style={styles.header}>ManDiligma</Text>
      <Text style={styles.subheader}>A mobile app for easy monitoring your halamans</Text>

      <View style={styles.grid}>
        <SensorCard title="Temperature" icon="thermometer" value={readings.temperature} unit="°C" min={-10} max={50} colorStart="#ff9a9e" colorEnd="#fecfef" />
        <SensorCard title="Humidity" icon="water-percent" value={readings.humidity} unit="%" colorStart="#89f7fe" colorEnd="#66a6ff" />
        <SensorCard title="Soil Moisture" icon="leaf" value={readings.soil} unit="%" colorStart="#a8ff78" colorEnd="#78ffd6" />
        <SensorCard title="pH Level" icon="flask" value={readings.ph} min={3} max={9} colorStart="#d299c2" colorEnd="#fef9d7" />
      </View>

      <View style={styles.historyCard}>
        <View style={styles.historyHeaderRow}>
          <Text style={styles.historyTitle}>Watering History</Text>
          <TouchableOpacity onPress={() => setModalVisible(true)}>
            <Text style={styles.viewAll}>View All</Text>
          </TouchableOpacity>
        </View>

        {readings.wateringHistory.slice(0, 3).map((time, index) => (
          <View key={index} style={styles.historyItem}>
            <MaterialCommunityIcons name="water" size={18} color="#9aa4b2" />
            <Text style={styles.historyText}>{time}</Text>
          </View>
        ))}
      </View>

      <Modal visible={modalVisible} transparent animationType="fade">
        <View style={styles.modalBackground}>
          <View style={styles.modalCard}>
            <Text style={styles.modalTitle}>Full Watering History</Text>
            <ScrollView style={{ maxHeight: 300 }}>
              {readings.wateringHistory.map((time, index) => (
                <View key={index} style={styles.modalItem}>
                  <MaterialCommunityIcons name="water" size={18} color="#9aa4b2" />
                  <Text style={styles.modalText}>{time}</Text>
                </View>
              ))}
            </ScrollView>
            <Pressable style={styles.closeBtn} onPress={() => setModalVisible(false)}>
              <Text style={styles.closeText}>Close</Text>
            </Pressable>
          </View>
        </View>
      </Modal>

      <View style={styles.footerSpace} />
    </ScrollView>
  );
}

const styles = StyleSheet.create({
  container: {
    padding: 16,
    paddingBottom: 40,
    backgroundColor: '#0f1724',
    minHeight: '100%',
  },
  header: {
    color: '#fff',
    fontSize: 28,
    fontWeight: '700',
    marginTop: 8,
  },
  subheader: {
    color: '#9aa4b2',
    marginTop: 6,
    marginBottom: 16,
  },
  grid: {
    flexDirection: 'row',
    flexWrap: 'wrap',
    justifyContent: 'space-between',
    gap: 12,
  },
  cardGradient: {
    width: (CARD_WIDTH - 12) / 2,
    borderRadius: 14,
    padding: 12,
    marginBottom: 12,
  },
  cardRow: {
    flexDirection: 'row',
    alignItems: 'center',
  },
  iconWrap: {
    width: 50,
    height: 50,
    borderRadius: 10,
    backgroundColor: 'rgba(255,255,255,0.12)',
    alignItems: 'center',
    justifyContent: 'center',
    marginRight: 10,
  },
  cardContent: {
    flex: 1,
  },
  cardTitle: {
    color: '#ffffffcc',
    fontSize: 13,
  },
  cardValue: {
    color: '#fff',
    fontSize: 20,
    fontWeight: '700',
    marginTop: 4,
  },
  miniBarBackground: {
    height: 6,
    borderRadius: 6,
    backgroundColor: 'rgba(0,0,0,0.18)',
    marginTop: 10,
    overflow: 'hidden',
  },
  miniBarFill: {
    height: '100%',
    borderRadius: 6,
    backgroundColor: 'rgba(255,255,255,0.9)',
  },
  historyCard: {
    marginTop: 16,
    backgroundColor: '#1e293b',
    borderRadius: 14,
    padding: 14,
  },
  historyHeaderRow: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
  },
  historyTitle: {
    color: '#dfe7f5',
    fontWeight: '700',
    fontSize: 16,
    marginBottom: 10,
  },
  viewAll: {
    color: '#60a5fa',
    fontSize: 14,
    fontWeight: '600',
  },
  historyItem: {
    flexDirection: 'row',
    alignItems: 'center',
    marginBottom: 6,
  },
  historyText: {
    color: '#9aa4b2',
    marginLeft: 8,
    fontSize: 14,
  },
  modalBackground: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    backgroundColor: 'rgba(0,0,0,0.6)',
  },
  modalCard: {
    width: '85%',
    backgroundColor: '#1e293b',
    borderRadius: 14,
    padding: 16,
    alignItems: 'stretch',
  },
  modalTitle: {
    color: '#dfe7f5',
    fontWeight: '700',
    fontSize: 18,
    marginBottom: 10,
  },
  modalItem: {
    flexDirection: 'row',
    alignItems: 'center',
    marginBottom: 6,
  },
  modalText: {
    color: '#9aa4b2',
    marginLeft: 8,
    fontSize: 14,
  },
  closeBtn: {
    marginTop: 14,
    backgroundColor: '#3b82f6',
    borderRadius: 8,
    paddingVertical: 8,
    alignItems: 'center',
  },
  closeText: {
    color: '#fff',
    fontWeight: '600',
  },
  footerSpace: {
    height: 40,
  },
});
