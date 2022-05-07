from fileinput import close
import numpy as np, scipy.stats as st
import linecache as lc
import matplotlib.pyplot as plt

def getInKb(value):
  if (value[-1] == 'B'):
    if (value[-2] == 'k'):
      return float(value[0:-2])

    if (value[-2] == 'M'):
      return float(value[0:-2]) * float(1000)

    return float(value[0:-1]) / float(1000)


def main():
  # No Clients calc
  
  finalValuesNoClientsCpu = []
  finalValuesNoClientsInput = []
  finalValuesNoClientsOutput = []

  confidenceIntervalsNoCpu = []
  confidenceIntervalsNoInput = []
  confidenceIntervalsNoOutput = []

  for sample in range(1, 15):
    noCpu = []
    noNetworkInput = []
    noNetworkOutput = []

    for j in range(1, 10):
      values = lc.getline("./with0Clients/sample" + str(j) + ".txt", sample).split()
      noCpu.append(float(values[0][0:-1]))
      noNetworkInput.append(getInKb(values[1]))
      noNetworkOutput.append(getInKb(values[2]))

    noCpuAverage = np.mean(noCpu)
    finalValuesNoClientsCpu.append(noCpuAverage)

    noNetworkInputAverage = np.mean(noNetworkInput)
    finalValuesNoClientsInput.append(noNetworkInputAverage)

    noNetworkOutputAverage = np.mean(noNetworkOutput)
    finalValuesNoClientsOutput.append(noNetworkOutputAverage)

    noCpuDeviation = st.sem(noCpu)
    noNetworkInputDeviation = st.sem(noNetworkInput)
    noNetworkOutputDeviation = st.sem(noNetworkOutput)

    confidenceIntervalsNoCpu.append(st.t.interval(0.95, len(noCpu) - 1, loc = noCpuAverage, scale = noCpuDeviation))
    confidenceIntervalsNoInput.append(st.t.interval(0.95, len(noNetworkInput) - 1, loc = noNetworkInputAverage, scale = noNetworkInputDeviation))
    confidenceIntervalsNoOutput.append(st.t.interval(0.95, len(noNetworkOutput) - 1, loc = noNetworkOutputAverage, scale = noNetworkOutputDeviation))
    

  # 100 Clients calc

  finalValuesHundredClientsCpu = []
  finalValuesHundredClientsInput = []
  finalValuesHundredClientsOutput = []

  confidenceIntervalsHundredCpu = []
  confidenceIntervalsHundredInput = []
  confidenceIntervalsHundredOutput = []

  for sample in range(1, 15):
    hundredCpu = []
    hundredNetworkInput = []
    hundredNetworkOutput = []
    
    for j in range(1, 10):
      values = lc.getline("./with100Clients/sample" + str(j) + ".txt", sample).split()
      hundredCpu.append(float(values[0][0:-1]))
      hundredNetworkInput.append(getInKb(values[1]))
      hundredNetworkOutput.append(getInKb(values[2]))

    hundredCpuAverage = np.mean(hundredCpu)
    finalValuesHundredClientsCpu.append(hundredCpuAverage)

    hundredNetworkInputAverage = np.mean(hundredNetworkInput)
    finalValuesHundredClientsInput.append(hundredNetworkInputAverage)

    hundredNetworkOutputAverage = np.mean(hundredNetworkOutput)
    finalValuesHundredClientsOutput.append(hundredNetworkOutputAverage)

    hundredCpuDeviation = st.sem(hundredCpu)
    hundredNetworkInputDeviation = st.sem(hundredNetworkInput)
    hundredNetworkOutputDeviation = st.sem(hundredNetworkOutput)

    confidenceIntervalsHundredCpu.append(st.t.interval(0.95, len(hundredCpu) - 1, loc = hundredCpuAverage, scale = hundredCpuDeviation))
    confidenceIntervalsHundredInput.append(st.t.interval(0.95, len(hundredNetworkInput) - 1, loc = hundredNetworkInputAverage, scale = hundredNetworkInputDeviation))
    confidenceIntervalsHundredOutput.append(st.t.interval(0.95, len(hundredNetworkOutput) - 1, loc = hundredNetworkOutputAverage, scale = hundredNetworkOutputDeviation))
  
  # 1000 Clients calc

  finalValuesThousandClientsCpu = []
  finalValuesThousandClientsInput = []
  finalValuesThousandClientsOutput = []

  confidenceIntervalsThousandCpu = []
  confidenceIntervalsThousandInput = []
  confidenceIntervalsThousandOutput = []

  for sample in range(1, 15):
    thousandCpu = []
    thousandNetworkInput = []
    thousandNetworkOutput = []
    
    for j in range(1, 10):
      values = lc.getline("./with1000Clients/sample" + str(j) + ".txt", sample).split()
      thousandCpu.append(float(values[0][0:-1]))
      thousandNetworkInput.append(getInKb(values[1]))
      thousandNetworkOutput.append(getInKb(values[2]))

    thousandCpuAverage = np.mean(thousandCpu)
    finalValuesThousandClientsCpu.append(thousandCpuAverage)

    thousandNetworkInputAverage = np.mean(thousandNetworkInput)
    finalValuesThousandClientsInput.append(thousandNetworkInputAverage)

    thousandNetworkOutputAverage = np.mean(thousandNetworkOutput)
    finalValuesThousandClientsOutput.append(thousandNetworkOutputAverage)

    thousandCpuDeviation = st.sem(thousandCpu)
    thousandNetworkInputDeviation = st.sem(thousandNetworkInput)
    thousandNetworkOutputDeviation = st.sem(thousandNetworkOutput)

    confidenceIntervalsThousandCpu.append(st.t.interval(0.95, len(thousandCpu) - 1, loc = thousandCpuAverage, scale = thousandCpuDeviation))
    confidenceIntervalsThousandInput.append(st.t.interval(0.95, len(thousandNetworkInput) - 1, loc = thousandNetworkInputAverage, scale = thousandNetworkInputDeviation))
    confidenceIntervalsThousandOutput.append(st.t.interval(0.95, len(thousandNetworkOutput) - 1, loc = thousandNetworkOutputAverage, scale = thousandNetworkOutputDeviation))



  values = finalValuesThousandClientsOutput
  confInterval = confidenceIntervalsThousandOutput
  title = "Uso de Memória (output) com 1000 clientes"
  ylabel = "kB"

  times = range(1, 15)
  fig, subplot = plt.subplots()
  # ax = plt.gca()
  # ax.set_ylim([-0.2, None])
  subplot.plot(times, values)
  ci0 = map(lambda x: x[0], confInterval)
  ci1 = map(lambda x: x[1], confInterval)
  subplot.fill_between(times, list(ci0), list(ci1), color='g', alpha=.2)
  plt.title(title)
  plt.ylabel(ylabel)
  plt.show()
  # plt.savefig("net-ouput-media")

  print("1000 clients")
  print("Means: ")
  print("CPU:")
  print("Input:")
  print("Output:")
  for j in range(0, 14):
    print(finalValuesThousandClientsOutput[j])

  print("Confindence intervals:")
  print("CPU:")
  print("Input:")
  print("Output:")
  for j in range(0, 14):
    print(confidenceIntervalsThousandOutput[j])
    



main()
