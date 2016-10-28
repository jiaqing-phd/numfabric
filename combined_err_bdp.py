#!/usr/bin/python
import sys
import pandas
import matplotlib as mp
import matplotlib.pyplot as plt
import seaborn as sns

plt.gcf().subplots_adjust(bottom=0.15)
mp.rcParams.update({"font.size":22})
bdp = (0.000016 * 10000000000.0)/8.0
#    bin_starts.append(i)

print("bdp is %f" %bdp)

def get_errors(fname):
    f = open(fname, "r")
    errors = {}
    for line in f:
        elems = (line.rstrip()).split(" ")
        if(elems[0] == "ERROR"):
            #if(float(elems[12]) > 3.0):
                errors[float(elems[2])]=((float(elems[4])), float(elems[10]))
    return errors



xf_err = get_errors(sys.argv[1])
dgd_err = get_errors(sys.argv[2])
rcp_err = get_errors(sys.argv[3])

flow_sizes = []
xf_errs = []
dgd_errs = []
rcp_errs =  []

for key in xf_err:
    if(key in dgd_err):
        if(key in rcp_err):
            dgd_errs.append((dgd_err[key])[0])
            rcp_errs.append((rcp_err[key])[0])
            flow_sizes.append((xf_err[key])[1])
            xf_errs.append((xf_err[key])[0])

xf_label=[]
for i in range(0, len(xf_errs)):
    xf_label.append("NUMFabric")

dgd_label=[]
for i in range(0, len(dgd_errs)):
    dgd_label.append("DGD")

rcp_label = []
for i in range(0, len(rcp_errs)):
    rcp_label.append("RCP$^\star$")


fig = plt.figure()
#ax.set_aspect(2)
df = pandas.DataFrame({"alg_err":xf_errs, "flow_sizes":flow_sizes, "alg_label":xf_label})
df1 = pandas.DataFrame({"alg_err":dgd_errs,"flow_sizes":flow_sizes, "alg_label":dgd_label})
df2 = pandas.DataFrame({"alg_err":rcp_errs,"flow_sizes":flow_sizes, "alg_label":rcp_label})
df = df.append(df1)
df = df.append(df2)
#df['size_quartiles'] = pandas.qcut(df["flow_sizes"], 10, labels=['0-10%','10-20%','20-30%','30-40%','40-50%','50-60%','60-70%','70-80%','80-90%','90-100%'])
#df['size_quartiles'] = pandas.cut(df["flow_sizes"], bin_starts, labels=['0-10 BDP','10-100 BDP','100-1000 BDP','1000-10000 BDP','10000-100000 BDP'])
#df['size_quartiles'] = pandas.cut(df["flow_sizes"], bin_starts, labels=['0-10 BDP','10-100 BDP','100-1000 BDP','1000-10000 BDP'])
#for i in (0, 10, 100,1000,10000,100000):
bin_starts = []
for i in (0, 5, 10, 100, 1000,10000,100000):
    bin_starts.append(i*bdp)
#df['size_quartiles'] = pandas.cut(df["flow_sizes"], bin_starts, labels=['(0-5 BDP)','(5-10 BDP)', '(10-100 BDP)', '(100-1K BDP)', '(1K-10K BDP)', '(10K-100K BDP)'])
#order_list=('(0-5 BDP)','(5-10 BDP)', '(10-100 BDP)', '(100-1K BDP)', '(1K-10K BDP)')
df['size_quartiles'] = pandas.cut(df["flow_sizes"], bin_starts, labels=['(0-5)','(5-10)', '(10-100)', '(100-1K)', '(1K-10K)', '(10K-100K)'])
order_list=('(0-5)','(5-10)', '(10-100)', '(100-1K)', '(1K-10K)')
df.to_csv("out_combined.csv",sep='\t')
print ("plotting... ")
#df.boxplot(column="alg_err", by='size_quartiles')
#kwargs={'showfliers':False}
#sns.set_color_codes()
#xlabels=['1','2','3','4','5']
sns.plt.figure(figsize=(12,9))
sns.set_style("darkgrid")
sns.set_context("notebook", font_scale=2.7, rc={"lines.linewidth": 2.5})
pal = {alg_label: "r" if alg_label == "NUMFabric" else "b" if alg_label == "DGD" else "g" for alg_label in df.alg_label.unique()}
#ax = sns.boxplot(x="size_quartiles", y="alg_err", hue="alg_label",data=df,palette=pal, order=order_list,**kwargs)
ax = sns.boxplot(x="size_quartiles", y="alg_err", hue="alg_label",data=df,palette=pal, order=order_list)
ax.set(xlabel="Flow Sizes in BDPs")
for tick in ax.xaxis.get_major_ticks():
    tick.label.set_fontsize(28)
ax.set(ylabel="Normalized deviation from ideal rates")
handles, labels = ax.get_legend_handles_labels()
sns.plt.legend(handles[0:3], labels[0:3])
#plt.ylim(-0.8, 0.8)
#plt.ylim(-0.4, 0.4)

#sns.plt.show()
sns.plt.savefig(sys.argv[4]+".png", bbox_inches='tight')
sns.plt.savefig(sys.argv[4]+".pdf", bbox_inches='tight')
plt.show()
print ("finished plotting.. ")

