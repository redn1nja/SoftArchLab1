FROM common as build
COPY . /usr/src/
WORKDIR /usr/src/
RUN cmake -DCMAKE_BUILD_TYPE=Release . && make

FROM build as final
EXPOSE $PORT
CMD ["sh", "-c", "/usr/src/message $PORT"]