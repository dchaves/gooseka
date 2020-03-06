FROM python:3.8.2-alpine3.11
ADD requirements.txt /
RUN pip install --upgrade pip && pip install -r /requirements.txt && rm /requirements.txt
RUN mkdir /mqtt_gateway
ADD mqtt_gateway.py /mqtt_gateway
WORKDIR /mqtt_gateway
CMD ["python3","/mqtt_gateway/mqtt_gateway.py"]